#include <nan.h>

#include "AnalyseGraph.h"
#include "ManipulateGraph.h"
#include "PullStep.h"
#include "PushStep.h"
#include "Tools.hpp"

using Nan::GetFunction;
using Nan::New;
using Nan::Set;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;

NAN_METHOD(PushFile);
NAN_METHOD(PullFile);
NAN_METHOD(AddFile);
NAN_METHOD(MovePart);
NAN_METHOD(RotatePart);

NAN_METHOD(EstimateDurationUpload);
NAN_METHOD(EstimateDurationDownload);
NAN_METHOD(GetProductHierarchy);

NAN_METHOD(ClearDatabase) {
    // Arguments
    // 0: DatabaseInfo (as json string)

    Stopwatch stopwatch;
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[0].As<v8::String>()));

    // init Graph
    Graph database(databaseInfo);
    // delete Graph
    database.deleteDatabase();

    stopwatch.stop();

    bool ret = true;
    info.GetReturnValue().Set(ret);
}

NAN_METHOD(PushFile) {
    // Arguments
    // 0: path to step file
    // 1: DatabaseInfo (as json string)

    Stopwatch stopwatch;

    std::string path = *Nan::Utf8String(info[0].As<v8::String>());
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[1].As<v8::String>()));

    std::cout << "Initialize database ..." << std::endl;

    // init Graph
    PushSTEP database(path, databaseInfo);

    // delete Graph
    database.deleteDatabase();
    bool ret = database.build();

    // build Graph
    if (!ret) {
        std::cout << "failed to initialize the database ..." << std::endl;
    }

    stopwatch.stop();

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(PullFile) {
    // Arguments
    // 0: outputdirectory
    // 1: DatabaseInfo (as json string)

    Stopwatch stopwatch;

    std::string outputPath = *Nan::Utf8String(info[0].As<v8::String>());
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[1].As<v8::String>()));

    std::cout << "Read database ... " << std::endl;

    PullSTEP database(outputPath, databaseInfo);
    int ret = database.writeStep();

    stopwatch.stop();

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(AddPart) {
    // Arguments
    // 0: path of the file to add
    // 1: part name
    // 2: assembly name
    // 3: DatabaseInfo (as json string)

    Stopwatch stopwatch;

    std::string path = *Nan::Utf8String(info[0].As<v8::String>());
    std::string part = *Nan::Utf8String(info[1].As<v8::String>());
    std::string assembly = *Nan::Utf8String(info[2].As<v8::String>());
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[3].As<v8::String>()));

    std::cout << "Add \"" + part + "\" to assembly" << std::endl;
    ManipulateGraph manipulate(databaseInfo);
    manipulate.addNewPart(path, part, assembly);

    bool ret = true;

    stopwatch.stop();

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(MovePart) {
    // Arguments
    // 0: name of the part to move
    // 1: new position of the part
    // 2: DatabaseInfo (as json string)

    Stopwatch stopwatch;

    std::string part = *Nan::Utf8String(info[0].As<v8::String>());
    std::string position = *Nan::Utf8String(info[1].As<v8::String>());
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[2].As<v8::String>()));

    Position pos = stringToPosition(position);
    std::cout << "Move \"" + part + "\" to position " + positionToString(pos)
              << std::endl;

    ManipulateGraph manipulate(databaseInfo);
    manipulate.movePart(part, pos);

    bool ret = true;

    stopwatch.stop();

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(RotatePart) {
    // Arguments
    // 0: name of the part to rotate
    // 1: new alignment of the part
    // 2: DatabaseInfo (as json string)

    Stopwatch stopwatch;

    std::string part = *Nan::Utf8String(info[0].As<v8::String>());
    std::string quaternion = *Nan::Utf8String(info[1].As<v8::String>());
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[2].As<v8::String>()));

    std::cout << "Rotate \"" + part + "\"" << std::endl;

    Quaternion quat = stringToQuaternion(quaternion);

    ManipulateGraph manipulate(databaseInfo);
    manipulate.rotatePart(part, quat);

    bool ret = true;

    stopwatch.stop();

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(EstimateDurationUpload) {
    // Arguments
    // 0: path to step file
    // 1: DatabaseInfo (as json string)

    std::string path = *Nan::Utf8String(info[0].As<v8::String>());
    STEPAnalyser stepAnalyser(path);
    double estimatedDuration = stepAnalyser.getNumEntities() * 0.024;
    info.GetReturnValue().Set(estimatedDuration);
}

NAN_METHOD(EstimateDurationDownload) {
    // Arguments
    // 0: DatabaseInfo (as json string)

    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[0].As<v8::String>()));

    GraphAnalyser graphAnalyser(databaseInfo);
    double estimatedDuration = graphAnalyser.getNumNodes() * 0.05;
    info.GetReturnValue().Set(estimatedDuration);
}

NAN_METHOD(GetProductHierarchy) {
    // Arguments
    // 0: DatabaseInfo (as json string)
    DatabaseInfo databaseInfo =
        JsonStringToDatabaseInfo(*Nan::Utf8String(info[0].As<v8::String>()));
    GraphAnalyser analyser(databaseInfo);

    info.GetReturnValue().Set(
        Nan::New(analyser.getProductHierarchyJson()).ToLocalChecked());
}

NAN_MODULE_INIT(InitAll) {
    Set(target, New<String>("clearDatabase").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(ClearDatabase)).ToLocalChecked());

    Set(target, New<String>("pushFile").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PushFile)).ToLocalChecked());

    Set(target, New<String>("pullFile").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PullFile)).ToLocalChecked());

    Set(target, New<String>("addPart").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(AddPart)).ToLocalChecked());

    Set(target, New<String>("movePart").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(MovePart)).ToLocalChecked());

    Set(target, New<String>("rotatePart").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(RotatePart)).ToLocalChecked());

    Set(target, New<String>("estimateDurationUpload").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(EstimateDurationUpload))
            .ToLocalChecked());

    Set(target, New<String>("estimateDurationDownload").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(EstimateDurationDownload))
            .ToLocalChecked());

    Set(target, New<String>("getProductHierarchy").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(GetProductHierarchy))
            .ToLocalChecked());
}

NODE_MODULE(graphstepAddon, InitAll)
