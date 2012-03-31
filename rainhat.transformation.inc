<?php

/**
 * @file
 * Callbacks file for the Transformation entity.
 */

/**
 * List callback.
 */
function rainhat_transformation_list() {
  $elements = array();

  // entity_load() called with one argument loads all entities of the specified type.
  $transformations = entity_load('rainhat_transformation');

  $header = array(t('Name'), t('Parent Fedora Connection'), t('Pid Prefix'), t('Hooked Type'), t('Datastreams (ID, Type)'));

  if (user_access('administer fedora connections transformations')) {
    $header[] = t('Administer');
  }

  // Sort the entries by date, since entity_load() returns them unsorted.
  uasort($transformations, create_function('$a,$b', 'return strcmp($a->name, $b->name);'));

  $rows = array();
  foreach ($transformations as $id => $transformation) {
    
    //Run off to the database and grab the name of the parent fedora connection
    $parent_connection = db_select('rainhat_fedora_connection', 'f')
      ->fields('f', array('name'))
      ->condition('id', $transformation->parent_fedora_connection_id)
      ->execute()
      ->fetchObject();
      
    //Grab the name of the hooked type
    $node_types = node_type_get_types();
    
    $row = array();
    $row[] = check_plain($transformation->name);
    $row[] = check_plain($parent_connection->name);
    $row[] = check_plain($transformation->pid_prefix);
    $row[] = check_plain($node_types[$transformation->hooked_type]->name);
    
    $datastreams = db_select('rainhat_datastream', 'f')
      ->fields('f')
      ->condition('parent_transformation_id', $transformation->id)
      ->execute();
    
    $datastream_list = "";
    
    foreach ($datastreams as $datastream){
      $datastream_list .= check_plain($datastream->datastream_id) .', '. check_plain($datastream->type);
      $datastream_list .= ' - '.l(t('Edit'), 'admin/structure/fedora/datastreams/'.$datastream->id.'/edit');
      $datastream_list .= ', '.l(t('Delete'), 'admin/structure/fedora/datastreams/'.$datastream->id.'/delete');
      $datastream_list .= '<br />';
    }
    $datastream_list .= l(t('Add New Datastream'), 'admin/structure/fedora/datastreams/'.$id.'/add');
    
    $row[] = $datastream_list;
   
    if (user_access('administer fedora connections transformations')) {
      $act = l(t('Edit'), "admin/structure/fedora/transformations/$id/edit");
      $act .= ', ';
      $act .= l(t('Delete'), "admin/structure/fedora/transformations/$id/delete");
      $row[] = $act;
    }

    $rows[] = $row;
  }
  $elements['transformations'] = array(
    '#theme' => 'table',
    '#title' => t('Transformations'),
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No Transformations...'),
  );
  return $elements;
}

/**
 * Add Transformation callback.
 */
function rainhat_transformation_add() {
  
  // Check to see if there is a parent fedora connection. If not, return the user to the fedora connections screen 
  // with an error. 
  $parent_connections = db_select('rainhat_fedora_connection', 'f')
    ->fields('f')
    ->execute()
    ->fetchAssoc();
  
  if (empty($parent_connections)){
    drupal_set_message(t('You must first create a Fedora Repository connection.'), 'error');
    drupal_goto('admin/structure/fedora/connections');
  }
  
  
  // Invoke the create() method we added to the controller to get a new entity with defaults.
  $transformation = entity_get_controller('rainhat_transformation')->create();

  return drupal_get_form('rainhat_transformation_form', $transformation);
}

/**
 * Transformation add/edit form callback.
 */
function rainhat_transformation_form($form, &$form_state, $transformation) {
  $form['#transformation'] = $transformation;
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => check_plain($transformation->name),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
        
  $form['pid_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Pid Prefix'),
    '#default_value' => check_plain($transformation->pid_prefix),
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
    '#default_value' => check_plain($transformation->parent_fedora_connection_id),
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
    '#default_value' => check_plain($transformation->hooked_type),
  ); 

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function rainhat_transformation_form_validate($form, &$form_state) {

  $transformation = $form['#transformation'];
  
  $id = 0;
  
  if (isset($transformation->id)){
    $id = $transformation->id;
  }
  
  //Ensure that the transformation name is unique
  $duplicates = db_select('rainhat_transformation', 'f')
    ->fields('f')
    ->condition('name', $form_state['values']['name'])
    ->condition('id', $id, '<>')
    ->execute()
    ->fetchAssoc();

  if (!empty($duplicates)) {
    form_set_error('', t('The name is not unique. Please choose another transformation name'));
  }
  
  //Check the pid prefix to ensure no weird characters. 
  if (preg_match("/^[a-z0-9]+$/i", $form_state['values']['pid_prefix']) == 0){
    form_set_error('pid_prefix', t('The pid prefix must have no spaces, and must only be made up of letters or numbers.'));
  }

}

/**
 * Transformation add/edit form submit callback.
 */
function rainhat_transformation_form_submit($form, &$form_state) {
  $transformation = $form['#transformation'];

  $transformation->name = $form_state['values']['name'];
  $transformation->pid_prefix = $form_state['values']['pid_prefix'];
  $transformation->parent_fedora_connection_id = $form_state['values']['parent_fedora_connection'];
  $transformation->hooked_type = $form_state['values']['hooked_type'];

  // Invoke the save() method that we added to the controller.
  entity_get_controller('rainhat_transformation')->save($transformation);
  drupal_set_message(t('Transformation saved.'));
  $form_state['redirect'] = 'admin/structure/fedora/transformations';
}

function rainhat_transformation_delete($form, &$form_state, $transformation) {
  $form['#transformation'] = $transformation;
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $transformation->id,
  );
  return confirm_form($form,
    t('Are you sure you want to delete @f?', array('@f' => $transformation->name)),
    '<front>',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_transformation_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $transformation = rainhat_transformation_load($form_state['values']['id']);

    // Invoke the delete() method that we added to the controller.
    entity_get_controller('rainhat_transformation')->delete(array($transformation->id));
    drupal_set_message(t('Transformation "@f" has been deleted.', array('@f' => $transformation->name)));
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
