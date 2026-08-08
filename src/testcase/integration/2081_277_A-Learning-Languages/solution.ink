// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var can = cpp_array(101, 101);

var comp = cpp_array(101);

var tot: dynamic;

func init(k: dynamic)
{
  {
    var i = 0;
    while ((i <= n))
    {
      comp[i] = i;
      i += 1;
    }
  }
  tot = (n - k);
}

func f(i: dynamic, j: dynamic)
{
  {
    var k = 0;
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

func find(i: dynamic)
{
  return if ((i == comp[i])) i else cpp_assign(comp[i], "=", find(comp[i]));
}

func join(i: dynamic, j: dynamic)
{
  if ((find(i) == find(j)))
  {
    return;
  }
  comp[find(i)] = find(j);
  tot -= 1;
}

func main()
{
  read(n, m);
  var a: dynamic;
  var b: dynamic;
  var c = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a);
      c += ((a == 0));
      {
        var j = 0;
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
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
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
  printf("%d\n", if ((c < n)) ((tot - 1) + (c)) else c);
}
