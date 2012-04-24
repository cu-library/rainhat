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
  
  $search_results = $fedoraConnection->searchByPIDPrefix($search, $token)->data;
  $results_xml_object = new SimpleXMLElement($search_results);
  
  foreach ($results_xml_object->resultList->objectFields as $object_field){
    $pid = (string) $object_field->pid;
    eval($node_import->php_script);  
  }
  
  if(isset($xml_object->listSession->token)){
    $token = (string) $xml_object->listSession->token;
    rainhat_node_import_perform_import($search, $fedoraConnection, $token, $node_import);
  }

}


