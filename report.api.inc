<?php

define('REPORT_FIELD_PARAM', 0);
define('REPORT_FIELD_COLUMN', 1);

function report_check_access($report) {
  if (user_access('report admin')) {
    return TRUE;
  }
  
  if (user_access('report access')) {
    return user_access('report access ' . $report->machine_name);
  }
  
  return FALSE;
}

function report_init_object() {
  $report = (object) array(
    'id' => 0,
    'machine_name' => '',
    'weight' => 0,
    'status' => 1,
    'validation' => '',
    'details' => array(),
  );
  
  $languages = language_list('enabled');
  foreach ($languages[1] as $lang => $data) {
    $report->details[$lang] = array(
      'language' => $lang,
      'title' => '',
    );
  }
  
  return $report;
}

function report_load($id) {
  static $reports = array();
  
  if (!isset($reports[$id])) {
    global $language;
    
    if (is_numeric($id)) {
      $report = db_fetch_object(db_query("SELECT * FROM {reports} WHERE id = %d", $id));
    }
    else {
      $report = db_fetch_object(db_query("SELECT * FROM {reports} WHERE machine_name = '%s'", $id));
    }
    
    if (!empty($report->id)) {
      $report->details = array();
      $result = db_query("SELECT * FROM {report_details} WHERE report_id = %d", $report->id);
      while ($data = db_fetch_array($result)) {
        $report->details[$data['language']] = $data;
      }
      
      if (isset($report->details[$language->language])) {
        $report->title = $report->details[$language->language]['title'];
      }
      
      $report->params = array();
      $report->columns = array();
      $params = array();
      $result = db_query("SELECT * FROM {report_params} WHERE report_id = %d ORDER BY weight ASC", $report->id);
      while ($data = db_fetch_array($result)) {
        $data['details'] = array();
        $params[$data['id']] = $data;
      }
      
      if (!empty($params)) {
        $result = db_query("SELECT * FROM {report_param_details} WHERE param_id IN (%s)", implode(', ', array_keys($params)));
        while ($data = db_fetch_array($result)) {
          $params[$data['param_id']]['details'][$data['language']] = $data;
        }
        
        foreach ($params as $data) {
          $data['title'] = $data['details'][$language->language]['title'];
          $data['description'] = $data['details'][$language->language]['description'];
          if ($data['param_type'] == REPORT_FIELD_PARAM) {
            $report->params[$data['id']] = $data;
          }
          else {
            $report->columns[$data['id']] = $data;
          }
        }
      }
      
      $report->conditions = array();
      $result = db_query("SELECT * FROM {report_conditions} WHERE report_id = %d", $report->id);
      while ($data = db_fetch_array($result)) {
        $report->conditions[$data['id']] = $data;
      }
      
      $reports[$report->id] = $report;
      $reports[$report->machine_name] = $report;
    }
    else {
      $reports[$id] = FALSE;
    }
  }
  
  return $reports[$id];
}

function report_save($report) {
  if (drupal_write_record('reports', $report, empty($report->id) ? array() : 'id') !== FALSE) {
    $languages = language_list('enabled');
    foreach ($languages[1] as $lang => $data) {
      $report->details[$lang]['language'] = $lang;
      $report->details[$lang]['report_id'] = $report->id;
      drupal_write_record(
        'report_details',
        $report->details[$lang],
        empty($report->details[$lang]['id']) ? array() : 'id'
      );
    }
    
    return TRUE;
  }
  
  return FALSE;
}

function report_delete($report) {
  db_query("DELETE FROM {reports} WHERE id = %d", $report->id);
  db_query("DELETE FROM {report_details} WHERE report_id = %d", $report->id);
  return TRUE;
}

function report_param_save($param) {
  if (drupal_write_record('report_params', $param, empty($param['id']) ? array() : 'id') !== FALSE) {
    $languages = language_list('enabled');
    foreach ($languages[1] as $lang => $data) {
      $param['details'][$lang]['language'] = $lang;
      $param['details'][$lang]['param_id'] = $param['id'];
      drupal_write_record(
        'report_param_details',
        $param['details'][$lang],
        empty($param['details'][$lang]['id']) ? array() : 'id'
      );
    }
    
    return TRUE;
  }
  
  return FALSE;
}

function report_param_delete($param) {
  db_query("DELETE FROM {report_params} WHERE id = %d", $param['id']);
  db_query("DELETE FROM {report_param_details} WHERE param_id = %d", $param['id']);
  return TRUE;
}

function report_get_list() {
  static $reports;
  
  if (!isset($reports)) {
    $reports = array();
    
    global $language;
    
    $sql = "SELECT r.*, rd.title FROM {reports} r
      LEFT JOIN {report_details} rd ON rd.report_id = r.id AND rd.language = '%s'";
    $result = db_query($sql, $language->language);
    while ($data = db_fetch_array($result)) {
      $reports[$data['machine_name']] = $data;
    }
  }
  
  return $reports;
}

function report_get_status_list() {
  return array(
    0 => t('Disabled'),
    1 => t('Enabled'),
  );
}

function report_get_param_type_list() {
  return array(
    'textfield'  => t('Textfield'),
    'date'       => t('Date'),
    'timestamp'  => t('Timestamp'),
    'select'     => t('Select'),
    'checkboxes' => t('Checkboxes'),
    'radios'     => t('Radios'),
    'checkbox'   => t('Checkbox'),
  );
}
