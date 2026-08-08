// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    swap(a, b);
  }
  while (b)
  {
    var r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / gcd(a, b));
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var e: dynamic;
  var f: dynamic;
  read(a, b, c, d, e, f);
  if (((b != 0) && (c != 0)))
  {
    var l = lcm(b, c);
    a *= (l / b);
    d *= (l / c);
    b = cpp_assign(c, "=", l);
    if (((d != 0) && (e != 0)))
    {
      var l = lcm(d, e);
      a *= (l / d);
      b *= (l / d);
      c *= (l / d);
      f *= (l / e);
      d = cpp_assign(e, "=", l);
      if ((a == 0))
      {
        write("Ron", "\n");
      } else
      {
        if ((f == 0))
        {
          write("Hermione", "\n");
        } else if ((f > a))
        {
          write("Ron", "\n");
        } else
        {
          write("Hermione", "\n");
        }
      }
    } else if (((d != 0) && (e == 0)))
    {
      if ((f != 0))
      {
        f = a;
        if ((d != 0))
        {
          write("Ron", "\n");
        } else
        {
          write("Hermione", "\n");
        }
      } else
      {
        if ((a == 0))
        {
          write("Ron", "\n");
        } else
        {
          write("Hermione", "\n");
        }
      }
    } else
    {
      write("Hermione", "\n");
    }
  } else if (((b == 0) && (c != 0)))
  {
    write("Hermione", "\n");
  } else
  {
    if ((d != 0))
    {
      write("Ron", "\n");
    } else
    {
      write("Hermione", "\n");
    }
  }
  return 0;
}
