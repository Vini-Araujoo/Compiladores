#include "codegen.h"

const char *traduzirTipoC(TipoDado tipo)
{
    switch (tipo)
    {
        case TIPO_BYTE:
            return "char";
        case TIPO_SHORT:
            return "short";
        case TIPO_INT:
            return "int";
        case TIPO_LONG:
            return "long long";
        case TIPO_FLOAT:
            return "float";
        case TIPO_DOUBLE:
            return "double";
        case TIPO_CHAR:
            return "char";
        case TIPO_BOOLEAN:
            return "bool";
        case TIPO_STRING:
            return "char *";
        case TIPO_VOID:
            return "void";
        case TIPO_SCANNER:
            return NULL;
        case TIPO_ARRAY:
        case TIPO_INDEFINIDO:
        default:
            return NULL;
    }
}
