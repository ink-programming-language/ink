// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var grafo = cpp_array(100100);

var root = [false];

var ferrou = false;

var wish = cpp_array(100100);

var cont = 0;

var flag = [false];

var val = 0;

func DFP(x: dynamic)
{
  {
    var it = grafo[x].begin();
    var fim = grafo[x].end();
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

func DFS(x: dynamic, last: dynamic)
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
    var it = grafo[x].begin();
    var fim = grafo[x].end();
    while ((it != fim))
    {
      DFS((*it), last);
      it += 1;
    }
  }
}

func main()
{
  scanf("%d %d", (&n), (&m));
  var x: dynamic;
  var y: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      scanf("%d %d", (&x), (&y));
      grafo[x].push_back(y);
      root[y] = true;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&wish[i]));
      i += 1;
    }
  }
  {
    var i = 1;
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
    var i = 1;
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
