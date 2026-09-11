// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func read() -> dynamic
{
  var n: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    n = (((n * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (n * f);
}

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  t = read();
  {
    var greg: dynamic = 1;
    while ((greg <= t))
    {
      d = read();
      k = read();
      flag = false;
      {
        var i: dynamic = 0;
        while ((i <= (d / k)))
        {
          if ((cpp_cast((sqrt(((((d * d)) / ((k * k))) - (i * i))))) == i))
          {
            flag = true;
            break;
          }
          i += 1;
        }
      }
      if ((flag == false))
      {
        printf("Ashish\n");
      } else
      {
        printf("Utkarsh\n");
      }
      greg += 1;
    }
  }
  return 0;
}
