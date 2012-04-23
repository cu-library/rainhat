<?php
/**
 * @file
 * This file contains the functions in which content from fedora is imported. 
 */

function rainhat_node_import_run_form($form, &$form_state, $node_import) {
  
  $form['#node_import'] = $node_import;

  $form['search'] = array(
    '#type' => 'textfield',
    '#title' => t('PID Search'),
    '#description' => t('The pid prefix to search for.'),
    '#size' => 40,
    '#maxlength' => 256,
    '#required' => TRUE,
    '#default_value' => '',
  );
  $form['next'] = array(
    '#type' => 'submit',
    '#value' => 'Import Nodes',
  );
  return $form;
}

function rainhat_node_import_run_form_submit($form, &$form_state) {
  $node_import = $form['#node_import'];

  //Perform the pid search
  $parent_fedora_connection = rainhat_fedora_connection_load($node_import->parent_fedora_connection_id);
  $fedoraConnection = new FedoraConnection($parent_fedora_connection->url, $parent_fedora_connection->admin_username, $parent_fedora_connection->admin_password);
  rainhat_node_import_perform_import($form_state['values']['search'], $fedoraConnection, NULL, $node_import);

  $form_state['redirect'] = '/admin/content';
}

function rainhat_node_import_perform_import($search, $fedoraConnection, $token, $node_import){
  
  $search_results = $fedoraConnection->searchByPIDPrefix($search, $token);
  $results_xml = $search_results->data;
  $results_xml_object = new SimpleXMLElement($results_xml);
  
  foreach ($results_xml_object->resultList->objectFields as $object_field){
    $pid = (string) $object_field->pid;
    
    $simple_field_imports = db_select('rainhat_field_import', 'rfi')
      ->fields('rfi')
      ->condition('parent_node_import_id',  $node_import->id)
      ->execute();
     
    $node = new stdClass(); 
    $node->type = $node_import->hooked_type;
    $node->language = LANGUAGE_NONE;
    $node->title = 'Node Created Programmatically By Rainhat on ' . date('c');
    global $user;
    $node->name = $user->name;
    node_object_prepare($node); 
    node_save($node);
        
    foreach ($simple_field_imports as $field_import){
      //First, process the scripted ingests
      if($field_import->type == 'scripted'){
        
        $new_field = eval($field_import->php_script);
        
        if($field_import->field_id === 'title' AND !empty($new_field)){
          $node->title = $new_field;
          continue;
        }
        
        if(is_array($new_field)){
          dpm('isarray');
          $field_id = $field_import->field_id;
          $node->$field_id = array($node->language => $new_field);
        }
        else{
          dpm($new_field);
          $field_id = $field_import->field_id;
          $node->$field_id = array($node->language => array($new_field));
          dpm($node);
        }
        
      }
      //Next, process the file ingests
      else{
        
      }
      
    }
    
    node_save($node);
    
  }
  
  if(isset($xml_object->listSession->token)){
    $token = (string) $xml_object->listSession->token;
    rainhat_node_import_perform_import($search, $fedoraConnection, $token, $node_import);
  }

}


