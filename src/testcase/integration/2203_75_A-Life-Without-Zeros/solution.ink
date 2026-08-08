// Translated from solution.cpp.

func getString(n: dynamic)
{
  var k: dynamic;
  while ((n != 0))
  {
    k += (((n % 10)) + cpp_char("0"));
    n /= 10;
  }
  reverse(k.begin(), k.end());
  return k;
}

func getSo(a: dynamic)
{
  var x = 0;
  {
    var i = 0;
    while ((i < a.length()))
    {
      x = ((x * 10) + ((a[i] - cpp_char("0"))));
      i += 1;
    }
  }
  return x;
}

func main()
{
  var a: dynamic;
  read(a);
  var b: dynamic;
  read(b);
  var k = (a + b);
  var a1 = getString(a);
  var b1 = getString(b);
  var c1 = getString(k);
  var a2: dynamic;
  var b2: dynamic;
  var c2: dynamic;
  {
    var i = 0;
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
    var i = 0;
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
    var i = 0;
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
  var x1 = getSo(a2);
  var x2 = getSo(b2);
  var x3 = getSo(c2);
  if (((x1 + x2) == x3))
  {
    write("YES", "\n");
  } else
  {
    write("NO", "\n");
  }
}
