<?php

/**
 * @file
 * Controller class definition file for Transformation entity.
 */

/**
 * Controller Class for Transformations
 */
class RainhatTransformationController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_transformation');
  }

  public function create() {   
    $transformation = new stdClass();
    $transformation->name = '';
    $transformation->pid_prefix= '';
    $transformation->parent_fedora_connection_id = 0;
    $transformation->hooked_type = '';
    return $transformation;
  }

  public function save($transformation) {
    if (empty($transformation->id)) {
           
      
      drupal_write_record('rainhat_transformation', $transformation);
      module_invoke_all('entity_insert', $transformation, 'rainhat_transformation');
      
      //Add the DC datastream to the new transformation
      $new_transformation = db_select('rainhat_transformation', 'f')
      ->fields('f')
      ->condition('name', $transformation->name)
      ->execute()
      ->fetchObject();

      $datastream = entity_get_controller('rainhat_datastream')->create($new_transformation->id);
      $datastream->datastream_id = 'DC';
      $datastream->type= 'XML';
      $datastream->php_script = 'return "<oai_dc:dc xmlns:oai_dc=\\"http://www.openarchives.org/OAI/2.0/oai_dc/\\" 
      xmlns:dc=\\"http://purl.org/dc/elements/1.1/\\" 
      xmlns:xsi=\\"http://www.w3.org/2001/XMLSchema-instance\\" 
      xsi:schemaLocation=\\"http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd\\">
      <dc:title>$node->title</dc:title>
      <dc:creator>$node->name</dc:creator>
      <dc:identifier>$pid</dc:identifier>
      </oai_dc:dc>";';
      $datastream->file_field = '';
      entity_get_controller('rainhat_datastream')->save($datastream);      
      
    }
    else {
      drupal_write_record('rainhat_transformation', $transformation, 'id');
      module_invoke_all('entity_update', $transformation, 'rainhat_transformation');
    }
    
    return $transformation;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $transformations = entity_load('rainhat_transformation', $ids);
    db_delete('rainhat_transformation')
      ->condition('id', $ids, 'IN')
      ->execute();

    // Make sure to invoke the delete hook for each topic.
    foreach ($transformations as $transformation) {
      module_invoke_all('entity_delete', $transformation, 'rainhat_transformation');
    }    
    
    $affected_datastreams = db_select('rainhat_datastream', 'f')
      ->fields('f', array('id'))
      ->condition('parent_transformation_id', $ids, 'IN')
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
