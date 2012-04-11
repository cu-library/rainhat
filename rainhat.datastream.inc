<?php

/**
 * @file
 * Callbacks file for the Datastream entity.
 */


/**
 * Add Datastream callback.
 */
function rainhat_datastream_add($node_export) {

  // Invoke the create() method we added to the controller to get a new entity with defaults.
  $datastream = entity_get_controller('rainhat_datastream')->create($node_export->id);

  return drupal_get_form('rainhat_datastream_form', $datastream);
}

/**
 * Datastream add/edit form callback.
 */
function rainhat_datastream_form($form, &$form_state, $datastream) {
  $form['#datastream'] = $datastream;
  
  $parent_node_export = db_select('rainhat_node_export', 'rne')
      ->fields('rne')
      ->condition('id', $datastream->parent_node_export_id)
      ->execute()
      ->fetchObject();

  $fields = field_info_instances('node', $parent_node_export->hooked_type);

  $form['datastream_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Datastream ID'),
    '#default_value' => check_plain($datastream->datastream_id),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Datastream Label'),
    '#default_value' => $datastream->label,
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
    '#description' => check_plain(t('A Label for this Datastream. This field can include php (for example, $node->title)')),
  );

  $form['type'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#default_value' => check_plain($datastream->type),
    '#options' => array('scripted' => 'Scripted (PHP returning XML or Text)', 'simple' => 'Simple (Field Uploaded without processing)'),
    '#description' => t('Is this datastream dynamically generated (scripted, mostly used to create XML documents), or simple (the field is uploaded directly, good for files or images).'),
  );


  $form['scripted'] = array(
    '#type' => 'fieldset',
    '#title' => t('Scripted Datastream Options'),
    '#states' => array(
      'visible' => array(
        ':input[name="type"]' => array('value' => 'scripted'),
      ),
    ),
  );

  $form['simple'] = array(
    '#type' => 'fieldset',
    '#title' => t('Simple Datastream Options'),
    '#states' => array(
      'visible' => array(
        ':input[name="type"]' => array('value' => 'simple'),
      ),
    ),
  );

  $header = array(
    array('data' => t('Field Name')),
    array('data' => t('Code'))
  );

  //First, add the title, author, and other nice fields
  $rows[] = array(
    array('data' => t('Title')),
    array('data' => t('$node->title'))
  );
  $rows[] = array(
    array('data' => t('Author')),
    array('data' => t('$node->name'))
  );

  foreach ($fields as $field) {
    $rows[] = array(
      array('data' => $field['label']),
      array('data' => "field_get_items('" . $field['entity_type'] . "', \$node, '" . $field['field_name'] . "')"),
    );
  }

  $form['scripted']['available_variables'] = array(
    '#markup' => theme('table', array('header' => $header, 'rows' => $rows, 'caption' => 'The type associated with this node export has the following fields:')),
  );

  $form['scripted']['php_script'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP Script'),
    '#default_value' => $datastream->php_script,
    '#rows' => 20,
  );

  $form['scripted']['control_group_scripted'] = array(
    '#type' => 'select',
    '#title' => t('Control Group'),
    '#default_value' => $datastream->control_group,
    '#options' => array('X' => t('Inline XML'), 'M' => t('Managed Content')),
  );

  $form['scripted']['mime_type_scripted'] = array(
    '#type' => 'textfield',
    '#title' => t('Mime Type'),
    '#default_value' => check_plain($datastream->mime_type),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_scripted"]' => array('value' => 'M'),
      ),
    ),
  );

  $form['simple']['control_group_simple'] = array(
    '#type' => 'select',
    '#title' => t('Control Group'),
    '#default_value' => $datastream->control_group,
    '#options' => array('M' => t('Managed Content'), 'E' => t('External Reference')),
  );

  $form['simple']['select_managed_location'] = array(
    '#type' => 'select',
    '#title' => t('Managed Content Location'),
    '#default_value' => $datastream->control_group,
    '#options' => array('f' => t('Node Field'), 'e' => t('URL')),
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'M'),
      ),
    ),
  );

  //Managed Content Options
  $form['simple']['file_field'] = array(
    '#type' => 'select',
    '#title' => t('Field to Upload'),
    '#default_value' => check_plain($datastream->file_field),
    '#options' => array_combine(array_keys($fields), array_keys($fields)),
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'M'),
        ':input[name="select_managed_location"]' => array('value' => 'f'),
      ),
    ),
  );

  $form['simple']['location_managed'] = array(
    '#type' => 'textfield',
    '#title' => t('Location'),
    '#default_value' => check_plain($datastream->location),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'M'),
        ':input[name="select_managed_location"]' => array('value' => 'e'),
      ),
    ),
  );

  $form['simple']['mime_type_simple_managed'] = array(
    '#type' => 'textfield',
    '#title' => t('Mime Type'),
    '#default_value' => check_plain($datastream->mime_type),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'M'),
        ':input[name="select_managed_location"]' => array('value' => 'e'),
      ),
    ),
  );

  //External Options Reference
  $form['simple']['location_external'] = array(
    '#type' => 'textfield',
    '#title' => t('Location'),
    '#default_value' => check_plain($datastream->location),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'E'),
      ),
    ),
  );

  $form['simple']['mime_type_simple_external'] = array(
    '#type' => 'textfield',
    '#title' => t('Mime Type'),
    '#default_value' => check_plain($datastream->mime_type),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => FALSE,
    '#states' => array(
      'visible' => array(
        ':input[name="control_group_simple"]' => array('value' => 'E'),
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

function rainhat_datastream_form_validate($form, &$form_state) {

  $datastream = $form['#datastream'];

  $id = 0;

  if (isset($datastream->id)) {
    $id = $datastream->id;
  }

  //Ensure that the DSID is unique
  $duplicates = db_select('rainhat_datastream', 'f')
    ->fields('f')
    ->condition('datastream_id', $form_state['values']['datastream_id'])
    ->condition('parent_node_export_id', $datastream->parent_node_export_id)
    ->condition('id', $id, '<>')
    ->execute()
    ->fetchObject();

  if ($duplicates !== FALSE) {
    form_set_error('datastream_id', t('The Datastream ID is not unique. Please choose another Datastream ID'));
  }

  //Check the pid prefix to ensure no weird characters.
  if (preg_match("/^[a-z0-9]+$/i", $form_state['values']['datastream_id']) == 0) {
    form_set_error('datastream_id', t('The datastream id must have no spaces, and must only be made up of letters or numbers.'));
  }

}

/**
 * Fedora collection add/edit form submit callback.
 */
function rainhat_datastream_form_submit($form, &$form_state) {
  $datastream = $form['#datastream'];

  $datastream->type = $form_state['values']['type'];
  $datastream->datastream_id = $form_state['values']['datastream_id'];
  $datastream->label = $form_state['values']['label'];

  if ($datastream->type == 'scripted') {
    $datastream->php_script = $form_state['values']['php_script'];
    $datastream->control_group =  $form_state['values']['control_group_scripted'];
    if ($datastream->control_group == 'M') {
      $datastream->mime_type = $form_state['values']['mime_type_scripted'];
    }
    else{
      $datastream->mime_type = 'text/xml';
    }
  }
  else {
    $datastream->control_group = $form_state['values']['control_group_simple'];
    if ($datastream->control_group == 'M') {
      if ($form_state['values']['select_managed_location'] == 'f') {
        $datastream->file_field = $form_state['values']['file_field'];
        $datastream->location = '';
        $datastream->mime_type = '';
      }
      else {
        $datastream->file_field = '';
        $datastream->location = $form_state['values']['location_managed'];
        $datastream->mime_type =  $form_state['values']['mime_type_simple_managed'];
      }
    }
    else {
      $datastream->location = $form_state['values']['location_external'];
      $datastream->mime_type =  $form_state['values']['mime_type_simple_external'];
    }
  }

  // Invoke the save() method that we added to the controller.
  entity_get_controller('rainhat_datastream')->save($datastream);
  drupal_set_message(t('Datastream saved.'));
  $form_state['redirect'] = 'admin/config/system/fedora/nodeexport';
}

function rainhat_datastream_delete($form, &$form_state, $datastream) {
  $form['#datastream'] = $datastream;

  return confirm_form($form,
    t('Are you sure you want to delete @f?', array('@f' => $datastream->datastream_id)),
    'admin/config/system/fedora/nodeexport',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function rainhat_datastream_delete_submit($form, &$form_state) {
  $datastream = $form['#datastream'];

  if ($form_state['values']['confirm']) {
    $datastream = rainhat_datastream_load($datastream->id);

    // Invoke the delete() method that we added to the controller.
    entity_get_controller('rainhat_datastream')->delete(array($datastream->id));
    drupal_set_message(t('datastream "@f" has been deleted.', array('@f' => $datastream->datastream_id)));
  }
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  else {
    $destination = array('destination' => 'admin/config/system/fedora/nodeexport');
  }
  $form_state['redirect'] = $destination['destination'];
}
