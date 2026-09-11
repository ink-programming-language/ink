// Translated from solution.cpp.

var N: dynamic = 100100;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(N);

var niv: dynamic = cpp_array(N);

var sol: dynamic = cpp_uninitialized();

var tata: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(N);

func dfs(nod: dynamic) -> dynamic
{
  if (sol)
  {
    return;
  }
  {
    var it: dynamic = v[nod].begin();
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

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
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
