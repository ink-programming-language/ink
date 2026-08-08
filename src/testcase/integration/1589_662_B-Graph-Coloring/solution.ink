// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var in_cpp = cpp_array(10);

var V: dynamic;

var M: dynamic;

class Edge
{
  var x: dynamic;
  var y: dynamic;
  var c: dynamic;
}

var E = cpp_array(101000);

var v = cpp_array(101000);

var T = cpp_array(101000);

var col: dynamic;

func BFS()
{
  var Q: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((v[i] == false))
      {
        var L: dynamic;
        Q.push(i);
        L.push_back(i);
        v[i] = true;
        while ((!Q.empty()))
        {
          var c = Q.front();
          Q.pop();
          {
            var it = V[c].begin();
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
        var ncol = 0;
        {
          var it = L.begin();
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
            var it = L.begin();
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

var Ans: dynamic;

var ans = false;

func check()
{
  var po = true;
  {
    var it = M.begin();
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
  var ncol = 0;
  {
    var i = 1;
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
      var i = 1;
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

func main()
{
  scanf("%d%d", (&n), (&m));
  V.resize((n + 100));
  var a: dynamic;
  var b: dynamic;
  {
    var i = 0;
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
    var i = 1;
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
