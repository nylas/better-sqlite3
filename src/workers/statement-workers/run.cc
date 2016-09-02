#include <sqlite3.h>
#include <nan.h>
#include "run.h"
#include "statement-worker.h"
#include "../../objects/statement/statement.h"
#include "../../util/macros.h"

RunWorker::RunWorker(Statement* stmt, sqlite3_stmt* handle, int handle_index, Nan::Callback* cb)
	: StatementWorker<Nan::AsyncWorker>(stmt, handle, handle_index, cb) {}
void RunWorker::Execute() {
	LOCK_DB(db_handle);
	int status = sqlite3_step(handle);
	if (status == SQLITE_DONE) {
		changes = sqlite3_changes(db_handle);
		id = sqlite3_last_insert_rowid(db_handle);
	} else if (status != SQLITE_ROW) {
		SetErrorMessage(sqlite3_errmsg(db_handle));
	} else {
		SetErrorMessage("Unexpected data returned by a write transaction.");
	}
	UNLOCK_DB(db_handle);
}
void RunWorker::HandleOKCallback() {
	Nan::HandleScope scope;
	
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::ForceSet(obj, Nan::New("changes").ToLocalChecked(), Nan::New<v8::Number>((double)changes));
	Nan::ForceSet(obj, Nan::New("id").ToLocalChecked(), Nan::New<v8::Number>((double)id));
	
	Resolve(obj);
}
