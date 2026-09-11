// Translated from solution.cpp.

var V: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d %d %d %d", (&n), (&k), (&a), (&b));
  var tka: dynamic = (a >= b);
  while ((a || b))
  {
    if (((tka && (a == 0)) || ((!tka) && (b == 0))))
    {
      printf("NO\n");
      return 0;
    }
    if (tka)
    {
      if ((a >= b))
      {
        var tk: dynamic = min(a, k);
        V.push_back(string_cpp(tk, cpp_char("G")));
        a -= tk;
      } else
      {
        V.push_back(string_cpp(1, cpp_char("G")));
        a -= 1;
      }
    } else if ((b >= a))
    {
      var tk: dynamic = min(b, k);
      V.push_back(string_cpp(tk, cpp_char("B")));
      b -= tk;
    } else
    {
      V.push_back(string_cpp(1, cpp_char("B")));
      b -= 1;
    }
    tka = (!tka);
  }
  {
    var i: dynamic = 0;
    while ((i < V.size()))
    {
      printf("%s", V[i].c_str());
      i += 1;
    }
  }
  printf("\n");
  return 0;
}
