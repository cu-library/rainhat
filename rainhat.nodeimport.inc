<?php

/**
 * @file
 * Callbacks file for the Node Import entity.
 */

/**
 * List callback.
 */
function rainhat_node_import_list() {
  $elements = array();

  // entity_load() called with one argument loads all entities of the specified type.
  $node_imports = entity_load('rainhat_node_import');

  $header = array(t('Name'), t('Parent Fedora Connection'), t('Hooked Type'), t('Fields'), t('Administer'));

  // Sort the entries alphabetically by name, since entity_load() returns them unsorted.
  uasort($node_imports, create_function('$a,$b', 'return strcmp($a->name, $b->name);'));

  $rows = array();
  foreach ($node_imports as $id => $node_import) {

    //Run off to the database and grab the name of the parent fedora connection
    $parent_connection = db_select('rainhat_fedora_connection', 'rfc')
      ->fields('rfc', array('name'))
      ->condition('id', $node_import->parent_fedora_connection_id)
      ->execute()
      ->fetchObject();

    //Grab the name of the hooked type
    $node_types = node_type_get_types();
    $hooked_type_name = $node_types[$node_import->hooked_type]->name;

    $row = array();
    $row[] = check_plain($node_import->name);
    $row[] = check_plain($parent_connection->name);
    $row[] = check_plain($hooked_type_name);

    //List the fields
    $fields = field_info_instances('node', $node_import->hooked_type);

    $field_list = "";
    foreach ($fields as $field) {
      $field_list .= $field['label'];
      $field_import = db_select('rainhat_field_import', 'rfi')
        ->fields('rfi')
        ->condition('parent_node_import_id', $node_import->id)
        ->condition('field_id', $field['field_name'])
        ->execute()
        ->fetchObject();
      if($field_import === FALSE){
        $field_list .= ' - ' . l(t('Add Field Import'), 'admin/structure/fedora/fieldimport/'.$id.'/'.$field['field_name'].'/add');
      } else {      
        $field_list .= ' - ' . l(t('Edit'), 'admin/structure/fedora/fieldimport/' . $field_import->id . '/edit');
        $field_list .= ', ' . l(t('Delete'), 'admin/structure/fedora/fieldimport/' . $field_import->id . '/delete');      
      }

      $field_list .= '<br />';
    }

    $row[] = $field_list;

    $act = l(t('Edit'), "admin/structure/fedora/nodeimport/$id/edit");
    $act .= ', ';
    $act .= l(t('Delete'), "admin/structure/fedora/nodeimport/$id/delete");
    $act .= ', ';
    $act .= l(t('Run Import'), "admin/structure/fedora/nodeimport/$id/run");
    $row[] = $act;

    $rows[] = $row;
  }
  $elements['nodeimport'] = array(
    '#theme' => 'table',
    '#title' => t('Node Imports'),
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No Node Imports...'),
  );
  return $elements;
}

/**
 * Add Node Import callback.
 */
function rainhat_node_import_add() {

  // Check to see if there is a parent fedora connection. If not, return the user to the fedora connections screen
  // with an error.
  $parent_connections = db_select('rainhat_fedora_connection', 'rfc')
    ->fields('rfc')
    ->execute()
    ->fetchObject();

  if ($parent_connections === FALSE) {
    drupal_set_message(t('You must first create a Fedora Repository connection.'), 'error');
    drupal_goto('admin/structure/fedora/connections');
  }

  $node_import = entity_get_controller('rainhat_node_import')->create();

  return drupal_get_form('rainhat_node_import_form', $node_import);
}

/**
 * Node Import add/edit form callback.
 */
function rainhat_node_import_form($form, &$form_state, $node_import) {
  $form['#node_import'] = $node_import;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('The name of the Node Import'),
    '#default_value' => check_plain($node_import->name),
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
    '#default_value' => check_plain($node_import->parent_fedora_connection_id),
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
    '#default_value' => check_plain($node_import->hooked_type),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function rainhat_node_import_form_validate($form, &$form_state) {

  $node_import = $form['#node_import'];

  $id = 0;

  if (isset($node_import->id)) {
    $id = $node_import->id;
  }

  //Ensure that the node import name is unique
  $duplicates = db_select('rainhat_node_import', 'rni')
    ->fields('rni')
    ->condition('name', $form_state['values']['name'])
    ->condition('id', $id, '<>')
    ->execute()
    ->fetchObject();

  if ($duplicates !== FALSE) {
    form_set_error('name', t('The name of this Node Import is not unique. Please choose another name.'));
  }

}

function rainhat_node_import_form_submit($form, &$form_state) {
  $node_import = $form['#node_import'];

  $node_import->name = $form_state['values']['name'];
  $node_import->parent_fedora_connection_id = $form_state['values']['parent_fedora_connection'];
  $node_import->hooked_type = $form_state['values']['hooked_type'];

  if (isset($node_import->id)) {
    drupal_set_message(t('Node Import successfully edited.'));
  }
  else {
    drupal_set_message(t('New Node Import saved.'));
  }
  
  entity_get_controller('rainhat_node_import')->save($node_import);

  $form_state['redirect'] = 'admin/structure/fedora/nodeimport';
}

function rainhat_node_import_delete($form, &$form_state, $node_import) {
  $form['#node_import'] = $node_import;
  return confirm_form($form,
    t('Are you sure you want to delete @name?', array('@name' => $node_import->name)),
    '/admin/structure/fedora/nodeimport',
    t('This action cannot be undone, and will delete the field imports associtated with this Node Import.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_node_import_delete_submit($form, &$form_state) {
  $node_import = $form['#node_import'];

  if ($form_state['values']['confirm']) {
    $node_import = rainhat_node_import_load($node_import->id);
    entity_get_controller('rainhat_node_import')->delete(array($node_import->id));
    drupal_set_message(t('Node import @n has been deleted.', array('@n' => $node_import->name)));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => '/admin/structure/fedora/nodeimport');
  }
  $form_state['redirect'] = $destination['destination'];
}

