<?php

/**
 * @file
 * Controller class definition file for Node Export entity.
 */

/**
 * Controller Class for the Node Export entitiy. 
 */
class RainhatNodeExportController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_node_export');
  }

  public function create() {   
    $node_export = new stdClass();
    $node_export->name = '';
    $node_export->pid_prefix= '';
    $node_export->parent_fedora_connection_id = 0;
    $node_export->hooked_type = '';
    return $node_export;
  }

  public function save($node_export) {
    if (empty($node_export->id)) {

      drupal_write_record('rainhat_node_export', $node_export);
      module_invoke_all('entity_insert', $node_export, 'rainhat_node_export');
      
      //Add a DC datastream to the new node export object
      $new_node_export = db_select('rainhat_node_export', 'rne')
      ->fields('rne')
      ->condition('name', $node_export->name)
      ->execute()
      ->fetchObject();

      $datastream = entity_get_controller('rainhat_datastream')->create($new_node_export->id);
      $datastream->datastream_id = 'DC';
      $datastream->type= 'scripted';
      $datastream->php_script = 'return "<oai_dc:dc xmlns:oai_dc=\\"http://www.openarchives.org/OAI/2.0/oai_dc/\\" 
      xmlns:dc=\\"http://purl.org/dc/elements/1.1/\\" 
      xmlns:xsi=\\"http://www.w3.org/2001/XMLSchema-instance\\" 
      xsi:schemaLocation=\\"http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd\\">
      <dc:title>$node->title</dc:title>
      <dc:creator>$node->name</dc:creator>
      <dc:identifier>$pid</dc:identifier>
      </oai_dc:dc>";';
      entity_get_controller('rainhat_datastream')->save($datastream);      
      
    }
    else {
      drupal_write_record('rainhat_node_export', $node_export, 'id');
      module_invoke_all('entity_update', $node_export, 'rainhat_node_export');
    }
    
    return $node_export;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $node_exports = entity_load('rainhat_node_export', $ids);
    db_delete('rainhat_node_export')
      ->condition('id', $ids, 'IN')
      ->execute();

    foreach ($node_exports as $node_export) {
      module_invoke_all('entity_delete', $node_export, 'rainhat_node_export');
    }    
    
    //Because datastreams live as children of the node export, we need
    // to delete them so they aren't 'orphaned' in the database. 
    // Though this sounds cruel, it's really for the best. 
    $affected_datastreams = db_select('rainhat_datastream', 'rd')
      ->fields('rd', array('id'))
      ->condition('parent_node_export_id', $ids, 'IN')
      ->execute();
    
    foreach ($affected_datastreams as $affected_datastream){
      entity_get_controller('rainhat_datastream')->delete(array($affected_datastream->id));
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
