<?php
/**
 * PHP patchset to upgrade from Genoa to Siena
 */

defined('COT_CODE') && defined('COT_UPDATE') or die('Wrong URL');

/**
 * r1318: datas/cache/templates cleanup for CoTemplate 2.5
 */

$dp = opendir('./datas/cache/templates');
while ($f = readdir($dp))
{
	$path = './datas/cache/templates/' . $f;
	if (is_file($path))
	{
		unlink($path);
	}
}
closedir($dp);

/**
 * r1463 Category order migration
 */
global $db_structure, $db_config;
$sql = $db->query("SELECT `structure_code`, `structure_order` FROM `$db_structure`");
$multi_insert = array();
while ($row = $sql->fetch())
{
	if ($row['structure_order'] != 'title.asc')
	{
		$order_parts = explode('.', $row['structure_order']);
		$multi_insert[] = array(
			'config_owner' => 'module',
			'config_cat' => 'page',
			'config_subcat' => $row['structure_code'],
			'config_order' => '01',
			'config_name' => 'order',
			'config_type' => COT_CONFIG_TYPE_CALLBACK,
			'config_value' => $order_parts[0],
			'config_default' => 'title',
			'config_variants' => 'cot_page_config_order()',
			'config_text' => ''
		);
		$multi_insert[] = array(
			'config_owner' => 'module',
			'config_cat' => 'page',
			'config_subcat' => $row['structure_code'],
			'config_order' => '02',
			'config_name' => 'way',
			'config_type' => COT_CONFIG_TYPE_SELECT,
			'config_value' => $order_parts[1],
			'config_default' => 'asc',
			'config_variants' => 'asc,desc',
			'config_text' => ''
		);
	}
}
$sql->closeCursor();

if (count($multi_insert) > 0)
{
	$db->insert($db_config, $multi_insert);
}

$db->query("ALTER TABLE `$db_structure` DROP COLUMN `structure_order`");

/**
 * r1473: move forum structure to general structure
 */

global $db_structure, $db_forum_stats, $db_forum_structure, $db_forum_sections, $db_x;
require_once cot_incfile('forums', 'module');

// Old forum table names, required for update
$db_forum_structure	= isset($db_forum_structure) ? $db_forum_structure : $db_x . 'forum_structure';
$db_forum_sections = isset($db_forum_sections) ? $db_forum_sections : $db_x . 'forum_sections';

$frm_structure_paths = array();
$sql = $db->query("SELECT * FROM `$db_forum_structure`");
while ($row = $sql->fetch())
{
	$db->query("INSERT INTO `$db_structure` (`structure_area`, `structure_code`, `structure_path`, `structure_tpl`, `structure_title`,
   `structure_desc`, `structure_icon`, `structure_locked`, `structure_count`) VALUES
	('forums', '".$row['fn_code']."', '".$row['fn_path']."', '', '".$row['fn_title']."', '".$row['fn_desc']."', '".$row['fn_icon']."', 0, 0)");

	$auth_permit = array(COT_GROUP_DEFAULT => 'RW', COT_GROUP_GUESTS => 'R', COT_GROUP_MEMBERS => 'RW');
	$auth_lock = array(COT_GROUP_DEFAULT => '0', COT_GROUP_GUESTS => 'A', COT_GROUP_MEMBERS => '0');
	cot_auth_add_item('forums', $row['fn_code'], $auth_permit, $auth_lock);

	$frm_structure_paths[$row['fn_code']] = $row['fn_path'];
}
$sql->closeCursor();

// Prefetch path parts
$frm_section_masters = array();
$frm_section_orders = array();
$sql = $db->query("SELECT `fs_id`, `fs_category`, `fs_masterid`, `fs_order` FROM `$db_forum_sections`");
while ($row = $sql->fetch())
{
	$frm_section_masters[$row['fs_id']] = $row['fs_masterid'];
	$frm_section_orders[$row['fs_id']] = $row['fs_order'];
}
$sql->closeCursor();

// Recursively calculates section path based on master order values
function frm_section_path($fs_id, &$frm_masters, &$frm_orders)
{
	if (isset($frm_masters[$fs_id]) && $frm_masters[$fs_id] > 0)
	{
		return frm_section_path($frm_masters[$fs_id], $frm_masters, $frm_orders)
			. '.' . $frm_orders[$fs_id];
	}
	else
	{
		return $frm_orders[$fs_id];
	}
}

$sql = $db->query("SELECT * FROM `$db_forum_sections`");
while ($row = $sql->fetch())
{
	$subpath = $frm_structure_paths[$row['fs_category']] . '.' . frm_section_path($row['fs_id'], $frm_section_masters, $frm_section_orders);
	$db->query("INSERT INTO `$db_structure` (`structure_area`, `structure_code`, `structure_path`, `structure_tpl`, `structure_title`,
   `structure_desc`, `structure_icon`, `structure_locked`, `structure_count`) VALUES
	('forums', '".$row['fs_id']."', '$subpath', '', '".$row['fs_title']."',
	'".$row['fs_desc']."', '".$row['fs_icon']."', '".$row['fs_state']."', '".$row['fs_fs_state']."')");

	$db->query("INSERT INTO `$db_forum_stats` (`fs_cat`, `fs_lt_id`, `fs_lt_title`, `fs_lt_date`, `fs_lt_posterid`, `fs_lt_postername`,
	`fs_topiccount`, `fs_postcount`, `fs_viewcount`) VALUES
	('".$row['fs_id']."', '".$row['fs_lt_id']."', '".$row['fs_lt_title']."', '".$row['fs_lt_date']."', '".$row['fs_lt_posterid']."', '".$row['fs_lt_postername']."',
	'".$row['fs_topiccount']."', '".$row['fs_postcount']."', '".$row['fs_viewcount']."')");
}
$sql->closeCursor();

$db->query("DROP TABLE IF EXISTS `$db_forum_sections`");
$db->query("DROP TABLE IF EXISTS `$db_forum_structure`");

?>