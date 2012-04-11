<?php

/**
 * @file
 * Callbacks file for the Field Import entity.
 */


/**
 * Add Field Import callback.
 */
function rainhat_field_import_add($node_export, $field) {

  // Invoke the create() method we added to the controller to get a new entity with defaults.
  $field_import = entity_get_controller('rainhat_field_import')->create($node_export->id, $field);

  return drupal_get_form('rainhat_field_import_form', $field_import);
}

/**
 * Field add/edit form callback.
 */
function rainhat_field_import_form($form, &$form_state, $field_import) {
  $form['#field_import'] = $field_import;
  
  $parent_node_import = db_select('rainhat_node_import', 'rni')
      ->fields('rni')
      ->condition('id', $field_import->parent_node_import_id)
      ->execute()
      ->fetchObject();
      
  $fields = field_info_instances('node', $parent_node_import->hooked_type);
  $field_label = $fields[$field_import->field_id]['label'];
  $node_types = node_type_get_types();
      
  $form['header'] = array(
    '#prefix' => '<h3>',
    '#suffix' => '</h3>', 
    '#markup' => t('Field Import for the @f field in node type @n:', array('@f' => $field_label, '@n'=>$node_types[$parent_node_import->hooked_type]->name)),
  );

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#default_value' => check_plain($field_import->type),
    '#options' => array('scripted' => 'Scripted (PHP taking XML and returning text)', 'file' => 'File'),
  );

  $form['php_script'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP Script'),
    '#default_value' => $field_import->php_script,
    '#rows' => 20,
    '#states' => array(
      'visible' => array(
        ':input[name="type"]' => array('value' => 'scripted'),
      ),
    ),
  );
  
  $form['datastream_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Datastream ID for the file you want to import to this field.'),
    '#default_value' => check_plain($field_import->datastream_id),
    '#size' => 50,
    '#maxlength' => 255,
    '#states' => array(
      'visible' => array(
        ':input[name="type"]' => array('value' => 'file'),
      ),
    ),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );


  return $form;
}

function rainhat_field_import_form_validate($form, &$form_state) {

  $field_import = $form['#field_import'];

}

/**
 * Fedora collection add/edit form submit callback.
 */
function rainhat_field_import_form_submit($form, &$form_state) {
  $field_import = $form['#field_import'];

  $field_import->type = $form_state['values']['type'];
  $field_import->php_script = $form_state['values']['php_script'];
  $field_import->datastream_id = $form_state['values']['datastream_id'];
  

  // Invoke the save() method that we added to the controller.
  entity_get_controller('rainhat_field_import')->save($field_import);
  drupal_set_message(t('Field Import saved.'));
  $form_state['redirect'] = 'admin/structure/fedora/nodeimport';
}

function rainhat_field_import_delete($form, &$form_state, $field_import) {
  $form['#field_import'] = $field_import;

  return confirm_form($form,
    t('Are you sure you want to delete this field import?'),
    'admin/structure/fedora/nodeimport',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_field_import_delete_submit($form, &$form_state) {
  $field_import = $form['#field_import'];

  if ($form_state['values']['confirm']) {
    $field_import = rainhat_field_import_load($field_import->id);

    // Invoke the delete() method that we added to the controller.
    entity_get_controller('rainhat_field_import')->delete(array($field_import->id));
    drupal_set_message(t('field import has been deleted.'));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => 'admin/structure/fedora/nodeimport');
  }
  $form_state['redirect'] = $destination['destination'];
}
