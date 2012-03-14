<?php

/**
 * @file
 * Callbacks file for the Fedora Collection entity.
 */

/**
 * List callback.
 */
function rainhat_fedoracollection_list() {
  $elements = array();

  // entity_load() called with one argument loads all entities of the specified type.
  $fedora_collections = entity_load('rainhat_fedora_collection');

  $header = array(t('Name'), t('Parent Fedora Connection'), t('Pid Prefix List'), t('Hooked Type'));

  if (user_access('administer fedora connections collections')) {
    $header[] = t('Administer');
  }

  // Sort the entries by date, since entity_load() returns them unsorted.
  uasort($fedora_collections, create_function('$a,$b', 'return strcmp($a->name, $b->name);'));

  $rows = array();
  foreach ($fedora_collections as $id => $fedora_collection) {
    
    //Run off to the database and grab the name of the parent fedora connection
    $parent_connection = db_select('rainhat_fedora_connection', 'f')
      ->fields('f', array('name'))
      ->condition('id', $fedora_collection->parent_fedora_connection_id)
      ->execute()
      ->fetchObject();
      
    //Grab the name of the hooked type
    $node_types = node_type_get_types();
    
    $row = array();
    $row[] = check_plain($fedora_collection->name);
    $row[] = check_plain($parent_connection->name);
    $row[] = check_plain($fedora_collection->pid_prefix_list);
    $row[] = check_plain($node_types[$fedora_collection->hooked_type]->name);

    if (user_access('administer fedora connections collections')) {
      $act = l(t('Edit'), "admin/structure/fedora/collections/$id/edit");
      $act .= ', ';
      $act .= l(t('Delete'), "admin/structure/fedora/collections/$id/delete");
      $row[] = $act;
    }

    $rows[] = $row;
  }
  $elements['fedora_collections'] = array(
    '#theme' => 'table',
    '#title' => t('Fedora collections'),
    '#header' => $header,
    '#rows' => $rows,
  );
  return $elements;
}

/**
 * Add Fedora collection callback.
 */
function rainhat_fedora_collection_add() {
  // Invoke the create() method we added to the controller to get a new entity with defaults.
  $fedora_collection = entity_get_controller('rainhat_fedora_collection')->create();

  return drupal_get_form('rainhat_fedora_collection_form', $fedora_collection);
}

/**
 * Fedora collection add/edit form callback.
 */
function rainhat_fedora_collection_form($form, &$form_state, $fedora_collection) {
  $form['#fedora_collection'] = $fedora_collection;

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => check_plain($fedora_collection->name),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
        
  $form['pid_prefix_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Pid Prefix List'),
    '#default_value' => check_plain($fedora_collection->pid_prefix_list),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  
  //Run off to the db and grab the possible parent connections
  $parent_connections = db_select('rainhat_fedora_connection', 'f')
    ->fields('f', array('id', 'name'))
    ->execute();
    
  $parent_fedora_connection_options = array();
  
  foreach ($parent_connections as $row) {
    $parent_fedora_connection_options[$row->id] = $row->name;
  }
  
  $form['parent_fedora_connection'] = array(
    '#type' => 'select',
    '#title' => t('Parent Fedora Connection'),
    '#options' => $parent_fedora_connection_options,
    '#default_value' => check_plain($fedora_collection->parent_fedora_connection_id),
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
    '#default_value' => check_plain($fedora_collection->hooked_type),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function rainhat_fedora_collection_form_validate($form, &$form_state) {

  //Ensure that the fedora collection is unique
  $duplicates = db_select('rainhat_fedora_collection', 'f')
    ->fields('f')
    ->condition('name', $form_state['values']['name'])
    ->execute();

  foreach ($duplicates as $duplicate) {
    form_set_error('', t('The name is not unique. Please choose another collection name'));
  }

}

/**
 * Fedora collection add/edit form submit callback.
 */
function rainhat_fedora_collection_form_submit($form, &$form_state) {
  $fedora_collection = $form['#fedora_collection'];

  $fedora_collection->name = $form_state['values']['name'];
  $fedora_collection->pid_prefix_list = $form_state['values']['pid_prefix_list'];
  $fedora_collection->parent_fedora_connection_id = $form_state['values']['parent_fedora_connection'];
  $fedora_collection->hooked_type = $form_state['values']['hooked_type'];

  // Invoke the save() method that we added to the controller.
  entity_get_controller('rainhat_fedora_collection')->save($fedora_collection);
  drupal_set_message(t('Fedora Collection saved.'));
  $form_state['redirect'] = 'admin/structure/fedora/collections';
}

function rainhat_fedora_collection_delete($form, &$form_state, $fedora_collection) {
  $form['#fedora_collection'] = $fedora_collection;
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $fedora_collection->id,
  );
  return confirm_form($form,
    t('Are you sure you want to delete @f?', array('@f' => $fedora_collection->name)),
    '<front>',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_fedora_collection_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $fedora_collection = rainhat_fedora_collection_load($form_state['values']['id']);

    // Invoke the delete() method that we added to the controller.
    entity_get_controller('rainhat_fedora_collection')->delete(array($fedora_collection->id));
    drupal_set_message(t('Fedora collection "@f" has been deleted.', array('@f' => $fedora_collection->name)));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => '<front>');
  }
  $form_state['redirect'] = $destination['destination'];
}
