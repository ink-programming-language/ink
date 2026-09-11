// Translated from solution.cpp.

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return lcm((-a), b);
  }
  if ((b < 0))
  {
    return lcm(a, (-b));
  }
  return (a * ((b / (gcd(a, b)))));
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < 0))
  {
    return gcd((-a), b);
  }
  if ((b < 0))
  {
    return gcd(a, (-b));
  }
  return  (((b == 0))) ? a : gcd(b, (a % b));
}

func deb(e: dynamic) -> dynamic
{
  write(e, "\n");
}

func deb(e1: dynamic, e2: dynamic) -> dynamic
{
  write(e1, " ", e2, "\n");
}

func deb(e1: dynamic, e2: dynamic, e3: dynamic) -> dynamic
{
  write(e1, " ", e2, " ", e3, "\n");
}

func deb(e1: dynamic, e2: dynamic, e3: dynamic, e4: dynamic) -> dynamic
{
  write(e1, " ", e2, " ", e3, " ", e4, "\n");
}

func deb(e1: dynamic, e2: dynamic, e3: dynamic, e4: dynamic, e5: dynamic) -> dynamic
{
  write(e1, " ", e2, " ", e3, " ", e4, " ", e5, "\n");
}

func deb(e1: dynamic, e2: dynamic, e3: dynamic, e4: dynamic, e5: dynamic, e6: dynamic) -> dynamic
{
  write(e1, " ", e2, " ", e3, " ", e4, " ", e5, " ", e6, "\n");
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  var res: dynamic = -1;
  var days: dynamic = cpp_array(100010);
  n = (cpp_expression("{ int a; scanf(\"%d\", &a); a; }"));
  x = (cpp_expression("{ int a; scanf(\"%d\", &a); a; }"));
  y = (cpp_expression("{ int a; scanf(\"%d\", &a); a; }"));
  {
    i = 1;
    while ((i <= n))
    {
      a = (cpp_expression("{ int a; scanf(\"%d\", &a); a; }"));
      days[i] = a;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      var success: dynamic = true;
      {
        j = i;
        while ((j <= max(1, (i - x))))
        {
          if ((days[j] < days[i]))
          {
            success = false;
          }
          j += 1;
        }
      }
      if ((!success))
      {
        i += 1;
        continue;
      }
      {
        j = i;
        while ((j <= min((i + y), n)))
        {
          if ((days[j] < days[i]))
          {
            success = false;
          }
          j += 1;
        }
      }
      if ((!success))
      {
        i += 1;
        continue;
      } else
      {
        res = i;
        break;
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
