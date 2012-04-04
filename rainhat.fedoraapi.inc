<?php

/**
 * @file
 * Contains the utility class to connect to Fedora.
 *
 * This class uses the REST API, as described here:
 * https://wiki.duraspace.org/display/FEDORA35/REST+API
 *
 * This class uses drupal_http_request, and as such, is dependent on Drupal.
 *
 */

class FedoraConnection {

  /**
   * Constructor for the Fedora object.
   */
  function __construct($url = 'http://localhost:8080/fedora/admin', $username = 'fedoraAdmin', $password = 'fedoraAdmin') {
    $this->url = $url . "/objects";
    $this->username = $username;
    $this->password = $password;
    $this->baseurl = $url;
  }

  /**
   * Check to see if a fedora repository exists and is usable using the url and username/password provided.
   *
   * @return bool
   */
  function check() {

    //Ingest and Purge a temporary object.
    $request = $this->createNewEmptyObject();

    if ($request->code != "201") {
      return FALSE;
    }
    else{
      $this->purgeObject($request->data);
      return TRUE;
    }
  }

  function createNewEmptyObject($pid = "new", $namespace = "rainhattest", $label = "rainhattest") {

    $options = array("namespace" => $namespace, "label" => $label);
    $url = $this->buildURL($options, $pid);

    $request_options = array(
      'method' => 'POST',
      'headers' => array("Authorization" => " Basic " . base64_encode($this->username . ':' . $this->password)),
    );

    return drupal_http_request($url, $request_options);
  }

  function purgeObject($pid) {

    $url = $this->buildURL(NULL, $pid);

    $request_options = array(
      'method' => 'DELETE',
      'headers' => array("Authorization" => " Basic " . base64_encode($this->username . ':' . $this->password)),
    );

    return drupal_http_request($url, $request_options);
  }

  function addDatastream($pid, $dsID, $controlGroup, $label, $data) {

    $url = $this->buildURL(array('controlGroup' => $controlGroup, 'label' => $label), $pid . '/datastreams/' . $dsID);
    $request_options = array(
      'method' => 'POST',
      'headers' => array("Authorization" => " Basic " . base64_encode($this->username . ':' . $this->password)),
      'data' => $data,
    );

    return drupal_http_request($url, $request_options);
  }


  /**
  * Utility function to build urls.
  */
  protected function buildURL($options=NULL, $method_or_pid = NULL) {

    $url = $this->url;
    if (!is_null($method_or_pid)) {
      $url .= "/" . $method_or_pid;
    }
    if (!is_null($options)) {
      foreach ($options as $key => $value) {
        if (is_bool($value)) {
          $string_value = ($value) ? 'true' : 'false';
          $options[$key] = $string_value;
        }
      }
      $url = $url . '?' . http_build_query($options, '', '&');
    }
    return $url;
  }

}


