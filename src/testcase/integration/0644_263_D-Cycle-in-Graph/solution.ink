// Translated from solution.cpp.

var N = 100100;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var p = cpp_array(N);

var niv = cpp_array(N);

var sol: dynamic;

var tata: dynamic;

var v = cpp_array(N);

func dfs(nod: dynamic)
{
  if (sol)
  {
    return;
  }
  {
    var it = v[nod].begin();
    while ((it != v[nod].end()))
    {
      if ((p[nod] != (*it)))
      {
        if ((!p[(*it)]))
        {
          p[(*it)] = nod;
          niv[(*it)] = (niv[nod] + 1);
          dfs((*it));
        } else
        {
          if (((niv[nod] - niv[(*it)]) >= k))
          {
            sol = nod;
            tata = (*it);
            return;
          }
        }
      }
      it += 1;
    }
  }
}

func main()
{
  var i: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(n, m, k);
  {
    i = 1;
    while ((i <= m))
    {
      read(a, b);
      v[a].push_back(b);
      v[b].push_back(a);
      i += 1;
    }
  }
  p[1] = -1;
  dfs(1);
  write(((niv[sol] - niv[tata]) + 1), "\n");
  while ((sol != tata))
  {
    write(sol, " ");
    sol = p[sol];
  }
  write(sol, "\n");
  return 0;
}
