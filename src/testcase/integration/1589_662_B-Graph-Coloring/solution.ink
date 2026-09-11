// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var in_cpp: dynamic = cpp_array(10);

var V: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

class Edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
}

var E: dynamic = cpp_array(101000);

var v: dynamic = cpp_array(101000);

var T: dynamic = cpp_array(101000);

var col: dynamic = cpp_uninitialized();

func BFS() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((v[i] == false))
      {
        var L: dynamic = cpp_uninitialized();
        Q.push(i);
        L.push_back(i);
        v[i] = true;
        while ((!Q.empty()))
        {
          var c: dynamic = Q.front();
          Q.pop();
          {
            var it: dynamic = V[c].begin();
            while ((it != V[c].end()))
            {
              if ((v[(*it)] == false))
              {
                v[(*it)] = true;
                L.push_back((*it));
                Q.push((*it));
                if ((M[pair(c, (*it))] == col))
                {
                  T[(*it)] = T[c];
                } else
                {
                  T[(*it)] = (((T[c] + 1)) % 2);
                }
              }
              it += 1;
            }
          }
        }
        var ncol: dynamic = 0;
        {
          var it: dynamic = L.begin();
          while ((it != L.end()))
          {
            if ((T[(*it)] == true))
            {
              ncol += 1;
            }
            it += 1;
          }
        }
        if (((L.size() - ncol) < ncol))
        {
          {
            var it: dynamic = L.begin();
            while ((it != L.end()))
            {
              T[(*it)] = (!T[(*it)]);
              it += 1;
            }
          }
        }
      }
      i += 1;
    }
  }
}

var Ans: dynamic = cpp_uninitialized();

var ans: dynamic = false;

func check() -> dynamic
{
  var po: dynamic = true;
  {
    var it: dynamic = M.begin();
    while (((it != M.end()) && (po == true)))
    {
      if ((it->second == col))
      {
        if (((T[it->first.first] + T[it->first.second]) == 1))
        {
          po = false;
        }
      } else
      {
        if (((T[it->first.first] + T[it->first.second]) != 1))
        {
          po = false;
        }
      }
      it += 1;
    }
  }
  if ((po == false))
  {
    return false;
  }
  var ncol: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((T[i] == true))
      {
        ncol += 1;
      }
      i += 1;
    }
  }
  if (((ncol < Ans.size()) || (ans == false)))
  {
    while ((!Ans.empty()))
    {
      Ans.pop();
    }
    ans = true;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        if ((T[i] == true))
        {
          Ans.push(i);
        }
        i += 1;
      }
    }
  }
  return true;
}

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  V.resize((n + 100));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d", (&a), (&b));
      scanf("%s", in_cpp);
      V[a].push_back(b);
      V[b].push_back(a);
      M[pair(a, b)] = cpp_assign(M[pair(b, a)], "=", ((in_cpp[0] == cpp_char("R"))));
      i += 1;
    }
  }
  col = false;
  BFS();
  check();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      T[i] = false;
      v[i] = false;
      i += 1;
    }
  }
  col = true;
  BFS();
  check();
  if ((ans == false))
  {
    puts("-1");
  } else
  {
    printf("%d\n", Ans.size());
    while ((!Ans.empty()))
    {
      printf("%d ", Ans.top());
      Ans.pop();
    }
    puts("");
  }
  return 0;
}
