// Translated from solution.cpp.

var zifu = cpp_array(20000);

var c: dynamic;

var fangxiang = [[1, 0], [0, 1], [-1, 0], [0, -1]];

var fangxiang2 = [[1, -1], [1, 1], [-1, 1], [-1, -1]];

func main()
{
  var n: dynamic;
  var a = cpp_array(300);
  read(n);
  c = getchar();
  gets(a);
  var zuida = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var temp = 0;
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
