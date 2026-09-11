// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var grafo: dynamic = cpp_array(100100);

var root: dynamic = [false];

var ferrou: dynamic = false;

var wish: dynamic = cpp_array(100100);

var cont: dynamic = 0;

var flag: dynamic = [false];

var val: dynamic = 0;

func DFP(x: dynamic) -> dynamic
{
  {
    var it: dynamic = grafo[x].begin();
    var fim: dynamic = grafo[x].end();
    while ((it != fim))
    {
      DFP((*it));
      it += 1;
    }
  }
  if (flag[x])
  {
    printf("%d\n", x);
  }
}

func DFS(x: dynamic, last: dynamic) -> dynamic
{
  if ((wish[x] == last))
  {
  } else if ((wish[x] == x))
  {
    last = wish[x];
    if ((!flag[wish[x]]))
    {
      val += 1;
      flag[wish[x]] = true;
    }
  } else
  {
    ferrou = true;
  }
  {
    var it: dynamic = grafo[x].begin();
    var fim: dynamic = grafo[x].end();
    while ((it != fim))
    {
      DFS((*it), last);
      it += 1;
    }
  }
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&m));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d %d", (&x), (&y));
      grafo[x].push_back(y);
      root[y] = true;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&wish[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!root[i]))
      {
        DFS(i, -1);
      }
      i += 1;
    }
  }
  if (ferrou)
  {
    write("-1", "\n");
    return 0;
  }
  write(val, "\n");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!root[i]))
      {
        DFP(i);
      }
      i += 1;
    }
  }
}
