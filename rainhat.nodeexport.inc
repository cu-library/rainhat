<?php

/**
 * @file
 * Callbacks file for the Node Export entity.
 */

/**
 * List callback.
 */
function rainhat_node_export_list() {
  $elements = array();

  // entity_load() called with one argument loads all entities of the specified type.
  $node_exports = entity_load('rainhat_node_export');

  $header = array(t('Name'), t('Parent Fedora Connection'), t('Hooked Type'), t('Pid Prefix'), t('Datastreams (ID, Type)'), t('Administer'));

  // Sort the entries alphabetically by name, since entity_load() returns them unsorted.
  uasort($node_exports, create_function('$a,$b', 'return strcmp($a->name, $b->name);'));

  $rows = array();
  foreach ($node_exports as $id => $node_export) {

    //Run off to the database and grab the name of the parent fedora connection
    $parent_connection = db_select('rainhat_fedora_connection', 'rfc')
      ->fields('rfc', array('name'))
      ->condition('id', $node_export->parent_fedora_connection_id)
      ->execute()
      ->fetchObject();

    //Grab the name of the hooked type
    $node_types = node_type_get_types();
    $hooked_type_name = $node_types[$node_export->hooked_type]->name;

    $row = array();
    $row[] = check_plain($node_export->name);
    $row[] = check_plain($parent_connection->name);
    $row[] = check_plain($hooked_type_name);
    $row[] = check_plain($node_export->pid_prefix);

    //Child datastream data
    $datastreams = db_select('rainhat_datastream', 'rd')
      ->fields('rd')
      ->condition('parent_node_export_id', $node_export->id)
      ->execute();

    $datastream_list = "";
    foreach ($datastreams as $datastream) {
      $datastream_list .= check_plain($datastream->datastream_id) . ', ' . ucfirst(check_plain($datastream->type));
      $datastream_list .= ' - ' . l(t('Edit'), 'admin/config/system/fedora/datastreams/' . $datastream->id . '/edit');
      $datastream_list .= ', ' . l(t('Delete'), 'admin/config/system/fedora/datastreams/' . $datastream->id . '/delete');
      $datastream_list .= '<br />';
    }

    $datastream_list .= l(t('Add New Datastream'), 'admin/config/system/fedora/datastreams/' . $id . '/add');

    $row[] = $datastream_list;

    $act = l(t('Edit'), "admin/config/system/fedora/nodeexport/$id/edit");
    $act .= ', ';
    $act .= l(t('Delete'), "admin/config/system/fedora/nodeexport/$id/delete");
    $row[] = $act;

    $rows[] = $row;
  }
  $elements['nodeexport'] = array(
    '#theme' => 'table',
    '#title' => t('Node Exports'),
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No Node Exports...'),
  );
  return $elements;
}

/**
 * Add Node Export callback.
 */
function rainhat_node_export_add() {

  // Check to see if there is a parent fedora connection. If not, return the user to the fedora connections screen
  // with an error.
  $parent_connections = db_select('rainhat_fedora_connection', 'rfc')
    ->fields('rfc')
    ->execute()
    ->fetchObject();

  if ($parent_connections === FALSE) {
    drupal_set_message(t('You must first create a Fedora Repository connection.'), 'error');
    drupal_goto('admin/config/system/fedora/connections');
  }

  $node_export = entity_get_controller('rainhat_node_export')->create();

  return drupal_get_form('rainhat_node_export_form', $node_export);
}

/**
 * Node Export add/edit form callback.
 */
function rainhat_node_export_form($form, &$form_state, $node_export) {
  $form['#node_export'] = $node_export;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('The name of the Node Export'),
    '#default_value' => check_plain($node_export->name),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  //Run off to the db and grab the possible parent connections
  $parent_connections = db_select('rainhat_fedora_connection', 'rfc')
    ->fields('rfc', array('id', 'name'))
    ->execute();

  $parent_fedora_connection_options = array();

  foreach ($parent_connections as $row) {
    $parent_fedora_connection_options[$row->id] = $row->name;
  }

  $form['parent_fedora_connection'] = array(
    '#type' => 'select',
    '#title' => t('Parent Fedora Connection'),
    '#options' => $parent_fedora_connection_options,
    '#default_value' => check_plain($node_export->parent_fedora_connection_id),
  );

  //Grab the possible types to hook into this collection
  $node_types = node_type_get_types();

  $hooked_type_options = array();

  foreach ($node_types as $type => $typeobj) {
    $hooked_type_options[$type] = $typeobj->name;
  }

  $form['hooked_type'] = array(
    '#type' => 'select',
    '#title' => t('Hooked Type'),
    '#options' => $hooked_type_options,
    '#default_value' => check_plain($node_export->hooked_type),
  );

  $form['pid_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Pid Prefix'),
    '#default_value' => check_plain($node_export->pid_prefix),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function rainhat_node_export_form_validate($form, &$form_state) {

  $node_export = $form['#node_export'];

  $id = 0;

  if (isset($node_export->id)) {
    $id = $node_export->id;
  }

  //Ensure that the node export name is unique
  $duplicates = db_select('rainhat_node_export', 'rne')
    ->fields('rne')
    ->condition('name', $form_state['values']['name'])
    ->condition('id', $id, '<>')
    ->execute()
    ->fetchObject();

  if ($duplicates !== FALSE) {
    form_set_error('name', t('The name of this Node Export is not unique. Please choose another name.'));
  }

  //Check the pid prefix to ensure no weird characters.
  if (preg_match("/^[a-z0-9]+$/i", $form_state['values']['pid_prefix']) == 0) {
    form_set_error('pid_prefix', t('The pid prefix must have no spaces, and must only be made up of letters or numbers.'));
  }

}

function rainhat_node_export_form_submit($form, &$form_state) {
  $node_export = $form['#node_export'];

  $node_export->name = $form_state['values']['name'];
  $node_export->pid_prefix = $form_state['values']['pid_prefix'];
  $node_export->parent_fedora_connection_id = $form_state['values']['parent_fedora_connection'];
  $node_export->hooked_type = $form_state['values']['hooked_type'];

  if (isset($node_export->id)) {
    drupal_set_message(t('Node Export successfully edited.'));
  }
  else {
    drupal_set_message(t('New Node Export saved.'));
  }
  
  entity_get_controller('rainhat_node_export')->save($node_export);

  $form_state['redirect'] = 'admin/config/system/fedora/nodeexport';
}

function rainhat_node_export_delete($form, &$form_state, $node_export) {
  $form['#node_export'] = $node_export;
  return confirm_form($form,
    t('Are you sure you want to delete @name?', array('@name' => $node_export->name)),
    '/admin/config/system/fedora/nodeexport',
    t('This action cannot be undone, and will delete the datastreams associtated with this Node Export.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_node_export_delete_submit($form, &$form_state) {
  $node_export = $form['#node_export'];

  if ($form_state['values']['confirm']) {
    $node_export = rainhat_node_export_load($node_export->id);
    entity_get_controller('rainhat_node_export')->delete(array($node_export->id));
    drupal_set_message(t('Node export @n has been deleted.', array('@n' => $node_export->name)));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => '/admin/config/system/fedora/nodeexport');
  }
  $form_state['redirect'] = $destination['destination'];
}
