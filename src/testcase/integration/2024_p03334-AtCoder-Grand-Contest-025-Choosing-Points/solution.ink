// Translated from solution.cpp.

var num: dynamic = cpp_array(4);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d1: dynamic = cpp_uninitialized();
  var d2: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&d1), (&d2));
  var num1: dynamic = 0;
  var num2: dynamic = 0;
  while ((((d1 & 1)) == 0))
  {
    num1 += 1;
    d1 >>= 1;
  }
  while ((((d2 & 1)) == 0))
  {
    num2 += 1;
    d2 >>= 1;
  }
  {
    var i: dynamic = 0;
    while ((i < ((n << 1))))
    {
      {
        var j: dynamic = 0;
        while ((j < ((n << 1))))
        {
          var tp1: dynamic = cpp_uninitialized();
          var tp2: dynamic = cpp_uninitialized();
          if ((num1 & 1))
          {
            tp1 = ((((i >> ((num1 >> 1)))) & 1));
          } else
          {
            tp1 = ((((((i >> ((num1 >> 1)))) + ((j >> ((num1 >> 1)))))) & 1));
          }
          if ((num2 & 1))
          {
            tp2 = ((((i >> ((num2 >> 1)))) & 1));
          } else
          {
            tp2 = ((((((i >> ((num2 >> 1)))) + ((j >> ((num2 >> 1)))))) & 1));
          }
          num[((tp1 + tp1) + tp2)] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  var wz: dynamic = cpp_uninitialized();
  {
    wz = 0;
    while ((wz < 4))
    {
      if ((num[wz] >= (n * n)))
      {
        break;
      }
      wz += 1;
    }
  }
  var rm: dynamic = (n * n);
  {
    var i: dynamic = 0;
    while ((i < ((n << 1))))
    {
      {
        var j: dynamic = 0;
        while ((j < ((n << 1))))
        {
          var tp1: dynamic = cpp_uninitialized();
          var tp2: dynamic = cpp_uninitialized();
          if ((num1 & 1))
          {
            tp1 = ((((i >> ((num1 >> 1)))) & 1));
          } else
          {
            tp1 = ((((((i >> ((num1 >> 1)))) + ((j >> ((num1 >> 1)))))) & 1));
          }
          if ((num2 & 1))
          {
            tp2 = ((((i >> ((num2 >> 1)))) & 1));
          } else
          {
            tp2 = ((((((i >> ((num2 >> 1)))) + ((j >> ((num2 >> 1)))))) & 1));
          }
          if ((((tp1 + tp1) + tp2) == wz))
          {
            printf("%d %d\n", i, j);
            rm -= 1;
            if ((rm == 0))
            {
              return 0;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
