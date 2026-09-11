// Translated from solution.cpp.

class edge
{
  var to: dynamic = cpp_uninitialized();
  var from_cpp: dynamic = cpp_uninitialized();
  func edge(to: dynamic, from_cpp: dynamic) -> dynamic
  {
      self->to = cpp_construct(to);
      self->from_cpp = cpp_construct(from_cpp);
    }
  func edge() -> dynamic
  {
    }
}

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var used: dynamic = cpp_array(5000);

var G: dynamic = cpp_array(555);

var E: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(N, M);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var s: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      read(s, t);
      s -= 1;
      t -= 1;
      E.push_back(edge(t, s));
      G[s].insert(i);
      G[t].insert(i);
      i += 1;
    }
  }
  if ((M == 0))
  {
    write(0, "\n");
    return 0;
  }
  var ed: dynamic = true;
  var res: dynamic = 0;
  while (ed)
  {
    var minid: dynamic = 0;
    while (G[minid].empty())
    {
      minid += 1;
    }
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        if (G[i].empty())
        {
          i += 1;
          continue;
        }
        if ((G[minid].size() > G[i].size()))
        {
          minid = i;
        }
        i += 1;
      }
    }
    res += (G[minid].size() - 1);
    {
      var it: dynamic = G[minid].begin();
      while ((it != G[minid].end()))
      {
        used[(*it)] = true;
        {
          var i: dynamic = 0;
          while ((i < N))
          {
            if ((i == minid))
            {
              i += 1;
              continue;
            }
            G[i].erase((*it));
            i += 1;
          }
        }
        it += 1;
      }
    }
    G[minid].clear();
    ed = false;
    {
      var i: dynamic = 0;
      while ((i < M))
      {
        if ((!used[i]))
        {
          ed = true;
        }
        i += 1;
      }
    }
  }
  write(res, "\n");
}
