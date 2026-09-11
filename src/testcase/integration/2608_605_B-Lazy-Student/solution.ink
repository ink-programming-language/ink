// Translated from solution.cpp.

var PI: dynamic = (2 * asin(1));

class Edge
{
  var w: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  func Edge(w: dynamic = 0, id: dynamic = 0, flag: dynamic = 0) -> dynamic
  {
      self->w = cpp_construct(w);
      self->id = cpp_construct(id);
      self->flag = cpp_construct(flag);
    }
  func operator_less(e: dynamic) -> dynamic
  {
      if ((w == e.w))
      {
        return (flag > e.flag);
      }
      return (w < e.w);
    }
}

var g_n: dynamic = cpp_uninitialized();

var g_m: dynamic = cpp_uninitialized();

var vb: dynamic = cpp_uninitialized();

var ve: dynamic = cpp_uninitialized();

var vpii_ans: dynamic = cpp_uninitialized();

func pretreat() -> dynamic
{
}

func input() -> dynamic
{
  read(g_n, g_m);
  if (cin.eof())
  {
    return false;
  }
  vb.clear();
  vb.resize((g_n + 1));
  ve.clear();
  ve.emplace_back();
  vpii_ans.clear();
  vpii_ans.resize((g_m + 1));
  {
    var i: dynamic = 1;
    var w: dynamic = cpp_uninitialized();
    var flag: dynamic = cpp_uninitialized();
    while ((i <= g_m))
    {
      scanf(" %d %d", (&w), (&flag));
      ve.emplace_back(w, i, flag);
      i += 1;
    }
  }
  return true;
}

func solve() -> dynamic
{
  sort((ve.begin() + 1), ve.end());
  var u: dynamic = 1;
  var v: dynamic = 3;
  var cnt: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= g_m))
    {
      var e: dynamic = ve[i];
      if ((e.flag == 1))
      {
        vpii_ans[e.id].first = cnt;
        vb[cnt] = true;
        vpii_ans[e.id].second = cpp_update(cnt, "++");
        vb[cnt] = true;
      } else
      {
        if ((vb[v] != true))
        {
          puts("-1");
          return;
        }
        vpii_ans[e.id].first = u;
        vpii_ans[e.id].second = v;
        u += 1;
        if ((u == (v - 1)))
        {
          u = 1;
          v += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= g_m))
    {
      printf("%d %d\n", vpii_ans[i].first, vpii_ans[i].second);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  pretreat();
  while (input())
  {
    solve();
  }
  return 0;
}
