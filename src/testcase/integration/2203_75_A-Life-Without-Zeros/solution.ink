// Translated from solution.cpp.

func getString(n: dynamic) -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  while ((n != 0))
  {
    k += (((n % 10)) + cpp_char("0"));
    n /= 10;
  }
  reverse(k.begin(), k.end());
  return k;
}

func getSo(a: dynamic) -> dynamic
{
  var x: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < a.length()))
    {
      x = ((x * 10) + ((a[i] - cpp_char("0"))));
      i += 1;
    }
  }
  return x;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  read(a);
  var b: dynamic = cpp_uninitialized();
  read(b);
  var k: dynamic = (a + b);
  var a1: dynamic = getString(a);
  var b1: dynamic = getString(b);
  var c1: dynamic = getString(k);
  var a2: dynamic = cpp_uninitialized();
  var b2: dynamic = cpp_uninitialized();
  var c2: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < a1.length()))
    {
      if ((a1[i] == cpp_char("0")))
      {
        i += 1;
        continue;
      }
      a2 += a1[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b1.length()))
    {
      if ((b1[i] == cpp_char("0")))
      {
        i += 1;
        continue;
      }
      b2 += b1[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < c1.length()))
    {
      if ((c1[i] == cpp_char("0")))
      {
        i += 1;
        continue;
      }
      c2 += c1[i];
      i += 1;
    }
  }
  var x1: dynamic = getSo(a2);
  var x2: dynamic = getSo(b2);
  var x3: dynamic = getSo(c2);
  if (((x1 + x2) == x3))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
}
