<?php

/**
 * @file
 * Callbacks file for the Datastream entity.
 */


/**
 * Add datastream callback.
 */
function rainhat_datastream_add($transformation) {
    
  // Invoke the create() method we added to the controller to get a new entity with defaults.
  $datastream = entity_get_controller('rainhat_datastream')->create($transformation->id);

  return drupal_get_form('rainhat_datastream_form', $datastream);
}

/**
 * datastream add/edit form callback.
 */
function rainhat_datastream_form($form, &$form_state, $datastream) {
  $form['#datastream'] = $datastream;
  
 
  $form['datastream_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Datastream ID'),
    '#default_value' => check_plain($datastream->datastream_id),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );
    
  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#default_value' => check_plain($datastream->type),
    '#options' => array('XML'=>'XML', 'FILE'=>'FILE'),
  );
  
  $form['php_script'] = array(
    '#type' => 'textarea',
    '#title' => t('Transformation Code (used if XML type)'),
    '#default_value' => $datastream->php_script,
  ); 
    
  $parent_transformation = db_select('rainhat_transformation', 'f')
      ->fields('f')
      ->condition('id', $datastream->parent_transformation_id)
      ->execute()
      ->fetchObject();
      
  $possible_file_fields = field_info_instances('node',$parent_transformation->hooked_type);
    
  $form['file_field'] = array(
    '#type' => 'select',
    '#title' => t('File Field (used if FILE type)'),
    '#default_value' => check_plain($datastream->file_field),
    '#options' => array_combine(array_keys($possible_file_fields), array_keys($possible_file_fields)),
  );
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function rainhat_datastream_form_validate($form, &$form_state) {

  $datastream = $form['#datastream'];
  
  $id = 0;
  
  if (isset($datastream->id)){
    $id = $datastream->id;
  }
  
  //Ensure that the DSID is unique
  $duplicates = db_select('rainhat_datastream', 'f')
    ->fields('f')
    ->condition('datastream_id', $form_state['values']['datastream_id'])
    ->condition('parent_transformation_id', $datastream->parent_transformation_id)
    ->condition('id', $id, '<>')
    ->execute()
    ->fetchAssoc();

  if (!empty($duplicates)) {
    form_set_error('datastream_id', t('The Datastream ID is not unique. Please choose another Datastream ID'));
  }
  
  //Check the pid prefix to ensure no weird characters. 
  if (preg_match("/^[a-z0-9]+$/i", $form_state['values']['datastream_id']) == 0){
    form_set_error('datastream_id', t('The datastream id must have no spaces, and must only be made up of letters or numbers.'));
  }

}

/**
 * Fedora collection add/edit form submit callback.
 */
function rainhat_datastream_form_submit($form, &$form_state) {
  $datastream = $form['#datastream'];
     
  $datastream->datastream_id = $form_state['values']['datastream_id'];
  $datastream->type = $form_state['values']['type'];

  // Invoke the save() method that we added to the controller.
  entity_get_controller('rainhat_datastream')->save($datastream);
  drupal_set_message(t('Datastream saved.'));
  $form_state['redirect'] = 'admin/structure/fedora/transformations';
}

function rainhat_datastream_delete($form, &$form_state, $datastream) {
  $form['#datastream'] = $datastream;
  $form['id'] = array(
    '#type' => 'value',
    '#value' => $datastream->id,
  );
  return confirm_form($form,
    t('Are you sure you want to delete @f?', array('@f' => $datastream->datastream_id)),
    '<front>',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_datastream_delete_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $datastream = rainhat_datastream_load($form_state['values']['id']);

    // Invoke the delete() method that we added to the controller.
    entity_get_controller('rainhat_datastream')->delete(array($datastream->id));
    drupal_set_message(t('datastream "@f" has been deleted.', array('@f' => $datastream->datastream_id)));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => 'admin/structure/fedora/transformations');
  }
  $form_state['redirect'] = $destination['destination'];
}
