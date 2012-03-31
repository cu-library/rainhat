<?php

/**
 * @file
 * Controller class definition file for Datastream entity.
 */

/**
 * Controller Class for Datastreams
 */
class RainhatDatastreamController extends DrupalDefaultEntityController {
  public function __construct($entityType) {
    parent::__construct('rainhat_datastream');
  }

  public function create($transformation_id) {
    $datastream = new stdClass();
    $datastream->datastream_id = '';
    $datastream->type= '';
    $datastream->php_script = '';
    $datastream->file_field = '';
    $datastream->parent_transformation_id = $transformation_id;
    return $datastream;
  }

  public function save($datastream) {
    if (empty($datastream->id)) {
      drupal_write_record('rainhat_datastream', $datastream);
      module_invoke_all('entity_insert', $datastream, 'rainhat_datastream');
    }
    else {
      drupal_write_record('rainhat_datastream', $datastream, 'id');
      module_invoke_all('entity_update', $datastream, 'rainhat_datastream');
    }

    return $datastream;
  }

  public function delete($ids) {
    if (empty($ids)) {
      return FALSE;
    }

    $datastreams = entity_load('rainhat_datastream', $ids);
    db_delete('rainhat_datastream')
      ->condition('id', $ids, 'IN')
      ->execute();

    // Make sure to invoke the delete hook for each topic.
    foreach ($datastreams as $datastream) {
      module_invoke_all('entity_delete', $datastream, 'rainhat_datastream');
    }

    // It's necessary to clear the caches when an entity is deleted.
    cache_clear_all();
    $this->resetCache();
    return TRUE;
  }
}
