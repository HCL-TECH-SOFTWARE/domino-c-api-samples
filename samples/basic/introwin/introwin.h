/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define FILE_GET_DB_TITLE	WM_USER+100
#define FILE_QUIT		WM_USER+110

#define	DBNAME_TEXT	100
#define	DBNAME_EDIT	101

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
//LRESULT FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL GetDBName(HWND, WORD, WPARAM, LPARAM);


