// Translated from solution.cpp.

var PI = (2 * asin(1));

class Edge
{
  var w: dynamic;
  var id: dynamic;
  var flag: dynamic;
  func Edge(w: dynamic = 0, id: dynamic = 0, flag: dynamic = 0)
  {
      this->w = cpp_construct(w);
      this->id = cpp_construct(id);
      this->flag = cpp_construct(flag);
    }
  func operator_less(e: dynamic)
  {
      if ((w == e.w))
      {
        return (flag > e.flag);
      }
      return (w < e.w);
    }
}

var g_n: dynamic;

var g_m: dynamic;

var vb: dynamic;

var ve: dynamic;

var vpii_ans: dynamic;

func pretreat()
{
}

func input()
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
    var i = 1;
    var w: dynamic;
    var flag: dynamic;
    while ((i <= g_m))
    {
      scanf(" %d %d", (&w), (&flag));
      ve.emplace_back(w, i, flag);
      i += 1;
    }
  }
  return true;
}

func solve()
{
  sort((ve.begin() + 1), ve.end());
  var u = 1;
  var v = 3;
  var cnt = 1;
  {
    var i = 1;
    while ((i <= g_m))
    {
      var e = ve[i];
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
    var i = 1;
    while ((i <= g_m))
    {
      printf("%d %d\n", vpii_ans[i].first, vpii_ans[i].second);
      i += 1;
    }
  }
}

func main()
{
  pretreat();
  while (input())
  {
    solve();
  }
  return 0;
}
