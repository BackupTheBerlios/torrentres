//////////////////////////////////////////////////
// Torment 1.0.1114
//////////////////////////////////////////////////
//
// Copyright © 2003 Dark Knight Software
// Programmed by Jason Zimmer
//
// Compiled in Visual Studio .NET 2003
//
// You are allowed to make changes to this code
// so long as all source and modified source
// along with any additional data is made
// available for public use.
//
// This License must also appear and be unchanged
// in all of the original source documents.
//
//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: StringTable.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
//////////////////////////////////////////////////
//
// This is a BitTorrent clone made in C++ (VS .NET 2003) 
// uses ATL and MFC libraries, so it is intended to work 
// on MS Operating Systems only.
// 
// It is a derivative work of Torment (Jason Zimmer), 
// but heavily enhanced and aimed to people who can understand Spanish 
// (though international releases and arbitrary translations 
// by XML additions are expected).
// 
// Both binary and source code forms are available as of version 0.0.2. 
// They're both working and ready to distribution.
// 
// This is a Work In Progress as of now. Main features expected are:
// - 'sparse' file creation, 
// - report arbitrary IP to Tracker, 
// - automatic prioritization of downloads, 
// - selective downloading within multifile torrents, 
// - selective downloading, 
// - multitracker specification, 
// - client recognition, and some more. 
// Some of them could be working yet by now, others are on their way.
// 
// Code can be used anywhere as long as this copyright notices 
// are kept untouched in both binary and sourcecode forms.
// 
// END OF COPYRIGHT NOTICE
//////////////////////////////////////////////////

#pragma once

#define STRTBL_YES									"Si"
#define STRTBL_NO									"No"

#define STRTBL_CLIENTSTATUS_TIMEDOUT				"Tiempo de espera agotado"
#define STRTBL_CLIENTSTATUS_CONNECTING				"Conectando"
#define STRTBL_CLIENTSTATUS_DOWNLOADING				"Bajando"
#define STRTBL_CLIENTSTATUS_UPLOADING				"Subiendo"
#define STRTBL_CLIENTSTATUS_IDLE					"Ocioso"

#define STRTBL_DATAGRAPH_IN							"Bajando"
#define STRTBL_DATAGRAPH_OUT						"Subiendo"

#define STRTBL_TORMENT_ERROR						"Error"
#define STRTBL_TORMENT_WARNING						"Aviso"
#define STRTBL_TORMENT_DUPLICATEINSTANCE			"Ya se está ejecutando."
#define STRTBL_TORMENT_TITLE						"TorrenTres"
#define STRTBL_TORMENT_RESIZEFILE					"%s no coincide el tamaño del archivo especificado en el torrent.\nSi continúas cambiarás el tamaño del archivo.\nSi el nuevo archivo es más pequeño es probable que se pierdan datos. ¿Quieres continuar?"
#define STRTBL_TORMENT_OPENWRITEFAILED				"Error abriendo '%s' para escritura"
#define STRTBL_TORMENT_BOGUSINSTANCE				"Error buscando la instancia de la aplicación."
#define STRTBL_TORMENT_SOCKETCREATEERROR			"Error al abrir el socket.\nNo será posible aceptar conexiones entrantes, lo que reducirá tu velocidad de bajada.\nIntenta cambiar el puerto en las opciones."
#define STRTBL_TORMENT_DISKFULL						"No hay espacio en disco para alojar '%s'."
#define STRTBL_TORMENT_UNKNOWNPREALLOC				"Hubo errores mientras se reservaba espacio en disco.\n Error #%d Motivo: '%s'\n¿Quieres continuar de todas formas?"
#define STRTBL_TORMENT_UNKNOWNRESIZE				"Hubo un error mientras se redimensionaba.\n Error #%d Motivo: '%s'\n¿Quieres continuar de todas formas?"

#define STRTBL_TORRENTLIST_FILE						"Archivo"
#define STRTBL_TORRENTLIST_PROGRESS					"Progreso"
#define STRTBL_TORRENTLIST_STATUS					"Estado"
#define STRTBL_TORRENTLIST_AVGBPSIN					"Bajada"
#define STRTBL_TORRENTLIST_AVGBPSOUT				"Subida"
#define STRTBL_TORRENTLIST_PEERSEEDS				"Semillas"
#define STRTBL_TORRENTLIST_PEERPARTIALS				"Chupones"
#define STRTBL_TORRENTLIST_PEERAVERAGEAVAILABLE		"Media disponible por usuario"
#define STRTBL_TORRENTLIST_ESTIMATEDCOMPLETIONTIME	"Finalizado en"
#define STRTBL_TORRENTLIST_BYTESSENT				"Bajados"
#define STRTBL_TORRENTLIST_BYTESRECEIVED			"Subidos"

#define STRTBL_TORRENT_FILEFILTER					"Torrentes (*.torrent)\0*.torrent\0Todos los archivos (*.*)\0*.*\0"
#define STRTBL_TORRENT_ALLFILESFILTER				"Todos los archivos (*.*)\0*.*\0"
#define STRTBL_TORRENT_FILEALREADYLOADED			"%s ya está cargado."
#define STRTBL_TORRENT_TORRENTLOADFAILED			"Error cargando '%s'\nMotivo: %s"
#define STRTBL_TORRENT_DESTINATIONFOLDER			"Selecciona la carpeta de destino para la descarga."
#define STRTBL_TORRENT_DESTINATIONFILE			    "Descargar a"

#define STRTBL_TORRENTINFO_FILENAME					"Nombre"
#define STRTBL_TORRENTINFO_FILELENGTH				"Tamaño"

#define STRTBL_TORRENTINFO_HEADERINFO				"Info"
#define STRTBL_TORRENTINFO_HEADERVALUE				"Valor"
#define STRTBL_TORRENTINFO_BATCHFILENAME			"Archivo %d Nombre"
#define STRTBL_TORRENTINFO_BATCHFILELENGTH			"Archivo %d Tamaño"
#define STRTBL_TORRENTINFO_TRACKERURL				"Tracker"
#define STRTBL_TORRENTINFO_INFOHASH					"Info Hash"
#define STRTBL_TORRENTINFO_BLOCKSIZE				"Tamaño del bloque"
#define STRTBL_TORRENTINFO_NUMBEROFBLOCKS			"Bloques"

#define STRTBL_CLIENTLIST_IP						"Ip"
#define STRTBL_CLIENTLIST_STATUS					"Estado"
#define STRTBL_CLIENTLIST_CHOKING					"Deteniendo"
#define STRTBL_CLIENTLIST_INTERESTED				"Interesado"
#define STRTBL_CLIENTLIST_BPSIN						"Bajando"
#define STRTBL_CLIENTLIST_BPSOUT					"Subiendo"
#define STRTBL_CLIENTLIST_CHOKED					"Detenido"
#define STRTBL_CLIENTLIST_INTERESTEDIN				"Interesado In"
#define STRTBL_CLIENTLIST_PERCENTAGE				"%"
#define STRTBL_CLIENTLIST_BYTESRECEIVED				"Bajados"
#define STRTBL_CLIENTLIST_BYTESSENT					"Subidos"
#define STRTBL_CLIENTLIST_CLIENT					"Cliente"

#define STRTBL_PROGRESSSTATUS_WAITING				"Esperando..."
#define STRTBL_PROGRESSSTATUS_ABORTED				"Abortado"
#define STRTBL_PROGRESSSTATUS_FINISHED				"Finalizado"
#define STRTBL_PROGRESSSTATUS_CHECKING				"Comprobando... (%.2f%%)"
#define STRTBL_PROGRESSSTATUS_PREALLOCATING			"Alojando..."
#define STRTBL_PROGRESSSTATUS_CONNECTING			"Conectado al Tracker..."
#define STRTBL_PROGRESSSTATUS_TRACKERFAILED			"Fallo Tracker: %s"
#define STRTBL_PROGRESSSTATUS_CONNECTINGTOPEERS		"Conectando a usuarios..."
#define STRTBL_PROGRESSSTATUS_NOBLOCKS				"El archivo no tiene bloques"
#define STRTBL_PROGRESSSTATUS_DOWNLOADING			"Bajando... (%.2f%%)"

#define STRTBL_PROPERTYSHEET_TITLE					"Opciones"

#define STRTBL_TORMENT_STATUSBPS					"%s Bajado / %s Subido"
#define STRTBL_TORMENT_STATUSAVGBPS					"%s Media bajado / %s Media subido"

#define STRTBL_CANTCALCULATE						"No se puede calcular"
#define STRTBL_MENU_FINISH							"&Finalizar descarga"
