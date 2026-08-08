// Translated from solution.cpp.

var int_cpp = dynamic;

func read()
{
  var n = 0;
  var f = 1;
  var ch = getchar();
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

func main()
{
  var t: dynamic;
  var d: dynamic;
  var k: dynamic;
  var flag: dynamic;
  t = read();
  {
    var greg = 1;
    while ((greg <= t))
    {
      d = read();
      k = read();
      flag = false;
      {
        var i = 0;
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
