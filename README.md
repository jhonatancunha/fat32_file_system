<h1 align="center">:rotating_light: Sistema de Arquivos FAT32  </h1>
<p align="center">
Implementação do sistema de arquivos FAT32 e de operações que manipulam os dados dentro do mesmo. Para tanto, foi utilizada a Linguagem de Programação C em conjunto com o compilador GCC na versão 9.4.0. O Sistema Operacional utilizado no processo foi o Linux Ubuntu 20.04.4 LTS.
</p>
<p align="center">
<img src="https://img.shields.io/github/repo-size/jhonatancunha/fat32_file_system" alt="">
<img src="https://img.shields.io/github/license/jhonatancunha/fat32_file_system" alt="">
<img src="https://img.shields.io/github/last-commit/jhonatancunha/fat32_file_system" alt="">
</p>
<br>

<br>

### :red_circle: Como utilizar

```bash
# Clone este repositório
$ git clone https://github.com/jhonatancunha/fat32_file_system

# Acesse a pasta do projeto no terminal/cmd
$ cd fat32_file_system

# Execute o makefile
$ make 

# Por fim inicie o executavel
$ ./main myimagefat32.img
```


## Funcionalidades

- **info:** exibe informações do disco e da FAT.
- **cluster <num>:** exibe o conteúdo do bloco num no formato texto.
- **pwd:** exibe o diretório corrente (caminho absoluto).
- **attr <file | dir>:** exibe os atributos de um arquivo (file) ou diretório (dir).
- **cd <path>:** altera o diretório corrente para o definido como path.
- **touch <file>:** cria o arquivo file com conteúdo vazio.
- **mkdir < dir >:** cria o diretório dir vazio.
- **rm <file>:** remove o arquivo file do sistema.
- **rmdir < dir >:** remove o diretório dir, se estiver vazio.
- **cp <source_path> <target_path>:** copia um arquivo de origem (source_path) para destino (target_path).
- **mv <source_path> <target_path>:** move um arquivo de origem (source_path) para destino (target_path).
- **rename <file> <newfilename>:** renomeia arquivo file para newfilename.
- **ls:** listar os arquivos e diretórios do diretório corrente.




### :mortar_board: Autores

<table><tr>
<td align="center"><a href="https://github.com/jhonatancunha">
 <img style="border-radius: 50%;" src="https://avatars0.githubusercontent.com/u/52831621?s=460&u=2b0cfdafeb7756176ded82c41738e773e92762b8&v=4" width="100px;" alt=""/>
<br />
 <b>Jhonatan Cunha</b></a>
 <a href="https://github.com/jhonatancunha" title="Repositorio Jhonatan"></a>



[![Gmail Badge](https://img.shields.io/badge/-jhonatancunha@alunos.utfpr.edu.br-c14438?style=flat-square&logo=Gmail&logoColor=white&link=mailto:jhonatancunha@alunos.utfpr.edu.br)](mailto:jhonatancunha@alunos.utfpr.edu.br)</td>

<td align="center"><a href="https://github.com/JessePires">
 <img style="border-radius: 50%;" src="https://avatars0.githubusercontent.com/u/20424496?s=460&u=87f2870ff153ab88402d6246cb3347a46ae33fe9&v=4" width="100px;" alt=""/>
<br />
 <b>Jessé Pires</b>
 </a> <a href="https://github.com/JessePires" title="Repositorio Jessé"></a>

[![Gmail Badge](https://img.shields.io/badge/-jesserocha@alunos.utfpr.edu.br-c14438?style=flat-square&logo=Gmail&logoColor=white&link=mailto:jesserocha@alunos.utfpr.edu.br)](mailto:jesserocha@alunos.utfpr.edu.br)</td>

<td align="center"><a href="https://github.com/gustavofavaro">
 <img style="border-radius: 50%;" src="https://avatars.githubusercontent.com/u/54089418?v=4" width="100px;" alt=""/>
<br />
 <b>Gustavo Favaro
</b>
 </a> <a href="https://github.com/gustavofavaro" title="Repositorio Gustavo"></a>

[![Gmail Badge](https://img.shields.io/badge/-gusfav@alunos.utfpr.edu.br-c14438?style=flat-square&logo=Gmail&logoColor=white&link=mailto:gusfav@alunos.utfpr.edu.br)](mailto:gusfav@alunos.utfpr.edu.br)</td>

</tr></table>

## :memo: Licença
[MIT](https://choosealicense.com/licenses/mit/)