// Translated from solution.cpp.

var zifu: dynamic = cpp_array(20000);

var c: dynamic = cpp_uninitialized();

var fangxiang: dynamic = [[1, 0], [0, 1], [-1, 0], [0, -1]];

var fangxiang2: dynamic = [[1, -1], [1, 1], [-1, 1], [-1, -1]];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(300);
  read(n);
  c = getchar();
  gets(a);
  var zuida: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var temp: dynamic = 0;
      while (((a[i] != cpp_char(" ")) && (i < n)))
      {
        if (((a[i] >= cpp_char("A")) && (a[i] <= cpp_char("Z"))))
        {
          temp += 1;
        }
        i += 1;
      }
      zuida = max(zuida, temp);
      i += 1;
    }
  }
  write(zuida, "\n");
  return 0;
}
