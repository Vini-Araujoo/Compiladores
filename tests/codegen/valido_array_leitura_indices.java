class ValidoArrayLeituraIndices {
    void executar() {
        int i = 1;
        int x = 0;
        vetor = new int[5];
        vetor[0] = 7;
        x = vetor[0];
        x = vetor[i];
        x = vetor[i + 1];
        vetor[i] = vetor[0];
    }
}
