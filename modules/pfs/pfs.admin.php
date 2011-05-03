<?php
/* ====================
[BEGIN_COT_EXT]
Hooks=admin
[END_COT_EXT]
==================== */

/**
 * Administration panel - PFS
 *
 * @package pfs
 * @version 0.1.0
 * @author Neocrome, Cotonti Team
 * @copyright Copyright (c) Cotonti Team 2008-2011
 * @license BSD
 */

(defined('COT_CODE') && defined('COT_ADMIN')) or die('Wrong URL.');

list($usr['auth_read'], $usr['auth_write'], $usr['isadmin']) = cot_auth('pfs', 'a');
cot_block($usr['isadmin']);

$s = cot_import('s', 'G', 'ALP');

if ($s == 'allpfs')
{
	require cot_incfile('pfs', 'module', 'admin.allpfs');
}
else
{
	$t = new XTemplate(cot_tplfile('pfs.admin', 'module'));

	$adminpath[] = array(cot_url('admin', 'm=other'), $L['Other']);
	$adminpath[] = array(cot_url('admin', 'm=pfs'), $L['PFS']);
	$adminhelp = $L['adm_help_pfs'];

	/* === Hook === */
	foreach (cot_getextplugins('pfs.admin.first') as $pl)
	{
		include $pl;
	}
	/* ===== */

	if (!function_exists('gd_info'))
	{
		$is_adminwarnings = true;
	}
	else
	{
		$gd_datas = gd_info();
		foreach ($gd_datas as $k => $i)
		{
			if (mb_strlen($i) < 2)
			{
				$i = $cot_yesno[$i];
			}
			$t->assign(array(
				'ADMIN_PFS_DATAS_NAME' => $k,
				'ADMIN_PFS_DATAS_ENABLE_OR_DISABLE' => $i
			));
			$t->parse('MAIN.PFS_ROW');
		}
	}

	$t->assign(array(
		'ADMIN_PFS_URL_CONFIG' => cot_url('admin', 'm=config&n=edit&o=module&p=pfs'),
		'ADMIN_PFS_URL_ALLPFS' => cot_url('admin', 'm=pfs&s=allpfs'),
		'ADMIN_PFS_URL_SFS' => cot_url('pfs', 'userid=0')
	));

	/* === Hook  === */
	foreach (cot_getextplugins('pfs.admin.tags') as $pl)
	{
		include $pl;
	}
	/* ===== */
}

$t->parse('MAIN');
$adminmain = $t->text('MAIN');

?>