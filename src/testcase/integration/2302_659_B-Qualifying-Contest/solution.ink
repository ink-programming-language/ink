// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  var r: dynamic;
  while ((b != 0))
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func sqr(x: dynamic)
{
  return (x * x);
}

func cube(x: dynamic)
{
  return ((x * x) * x);
}

class data
{
  var name: dynamic;
  var reg: dynamic;
  var score: dynamic;
}

var a = cpp_array(int_cpp((1e5 + 100)));

func cmp(a: dynamic, b: dynamic)
{
  return ((a.reg < b.reg) || (((a.reg == b.reg) && (a.score > b.score))));
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = (0);
    while ((i <= ((n - 1))))
    {
      read(a[i].name, a[i].reg, a[i].score);
      i += 1;
    }
  }
  sort(a, (a + n), cmp);
  var meReg = 0;
  {
    var i = (0);
    while ((i <= ((n - 1))))
    {
      if ((meReg == a[i].reg))
      {
        i += 1;
        continue;
      }
      meReg = a[i].reg;
      if ((i == (n - 2)))
      {
        write(a[i].name, " ", a[(i + 1)].name, "\n");
        i += 1;
        continue;
      }
      if (((a[(i + 1)].score <= a[(i + 2)].score) && (a[(i + 1)].reg == a[(i + 2)].reg)))
      {
        write("?", "\n");
        i += 1;
        continue;
      }
      write(a[i].name, " ", a[(i + 1)].name, "\n");
      i += 1;
    }
  }
}
