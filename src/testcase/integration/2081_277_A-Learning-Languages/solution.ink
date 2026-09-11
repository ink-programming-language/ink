// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var can: dynamic = cpp_array(101, 101);

var comp: dynamic = cpp_array(101);

var tot: dynamic = cpp_uninitialized();

func init(k: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      comp[i] = i;
      i += 1;
    }
  }
  tot = (n - k);
}

func f(i: dynamic, j: dynamic) -> dynamic
{
  {
    var k: dynamic = 0;
    while ((k < m))
    {
      if ((can[i][k] && can[j][k]))
      {
        return true;
      }
      k += 1;
    }
  }
  return false;
}

func find(i: dynamic) -> dynamic
{
  return  ((i == comp[i])) ? i : cpp_assign(comp[i], "=", find(comp[i]));
}

func join(i: dynamic, j: dynamic) -> dynamic
{
  if ((find(i) == find(j)))
  {
    return;
  }
  comp[find(i)] = find(j);
  tot -= 1;
}

func main() -> dynamic
{
  read(n, m);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a);
      c += ((a == 0));
      {
        var j: dynamic = 0;
        while ((j < a))
        {
          read(b);
          b -= 1;
          can[i][b] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  init(c);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if (f(i, j))
          {
            join(i, j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n",  ((c < n)) ? ((tot - 1) + (c)) : c);
}
