// Translated from solution.cpp.

func mini(a4: dynamic, b4: dynamic)
{
  a4 = min(a4, b4);
}

func maxi(a4: dynamic, b4: dynamic)
{
  a4 = max(a4, b4);
}

func operator_shift_left(out: dynamic, pair: dynamic)
{
  return (((((out << "(") << pair.first) << ", ") << pair.second) << ")");
}

class SegmentTree
{
  var n: dynamic;
  var to_add: dynamic;
  var T: dynamic;
  func SegmentTree(n: dynamic)
  {
      n = 1;
      while ((n < n))
      {
        n *= 2;
      }
      T = vector((2 * n));
    }
  func add(x: dynamic, y: dynamic, id: dynamic)
  {
      to_add.push_back(make_pair(x, pr(y, id)));
    }
  func query(i: dynamic, bb: dynamic, ee: dynamic, b: dynamic, e: dynamic, y1: dynamic, y2: dynamic, res: dynamic)
  {
      if (((ee <= b) || (e <= bb)))
      {
        return;
      }
      if (((b <= bb) && (ee <= e)))
      {
        T[i].single(y1, y2, res);
        return;
      }
      query((2 * i), bb, (((bb + ee)) / 2), b, e, y1, y2, res);
      query(((2 * i) + 1), (((bb + ee)) / 2), ee, b, e, y1, y2, res);
    }
  func query(b: dynamic, e: dynamic, y1: dynamic, y2: dynamic, res: dynamic)
  {
      query(1, 0, n, b, e, y1, y2, res);
    }
  func init()
  {
      sort((to_add).begin(), (to_add).end());
      for (var tr in to_add)
      {
        T[(n + tr.first)].v.push_back(tr.second);
      }
      {
        var i = ((n - 1));
        while ((i >= (1)))
        {
          T[i].v.resize((T[(2 * i)].v.size() + T[((2 * i) + 1)].v.size()));
          merge((T[(2 * i)].v).begin(), (T[(2 * i)].v).end(), (T[((2 * i) + 1)].v).begin(), (T[((2 * i) + 1)].v).end(), T[i].v.begin());
          i -= 1;
        }
      }
      {
        var i = 0;
        while ((i < ((2 * n))))
        {
          T[i].b = 0;
          T[i].e = ((cpp_cast((T[i].v).size())) - 1);
          i += 1;
        }
      }
    }
}

class Tree
{
  var n: dynamic;
  var T: dynamic;
  var S: dynamic;
  var edg: dynamic;
  func Tree(n: dynamic)
  {
      this->n = cpp_construct(n);
      this->T = cpp_construct(n);
      this->S = cpp_construct((2 * n));
    }
  func add_edge(a: dynamic, b: dynamic)
  {
      edg.push_back(make_pair(b, a));
      T[b].ngb.push_back(a);
      T[a].ngb.push_back(b);
    }
  func dfs(t: dynamic, i: dynamic = 0, par: dynamic = -1)
  {
      var v = T[i];
      v.par = par;
      v.in_cpp = t;
      t += 1;
      for (var j in v.ngb)
      {
        if ((j != par))
        {
          dfs(t, j, i);
        }
      }
      v.out = t;
      t += 1;
      return;
    }
  func insert(i: dynamic, j: dynamic, v: dynamic)
  {
      if ((T[i].out > T[j].out))
      {
        swap(i, j);
      }
      S.add(T[i].out, T[j].out, v);
      S.add(T[j].out, T[i].out, v);
    }
  func get(id: dynamic)
  {
      var res: dynamic;
      var p = edg[id];
      var i = p.first;
      if ((T[p.first].par != p.second))
      {
        i = p.second;
      }
      S.query(T[i].in_cpp, (T[i].out + 1), T[i].in_cpp, T[i].out, res);
      return res;
    }
}

func main()
{
  ios_base.sync_with_stdio(false);
  write(fixed, setprecision(10));
  var n: dynamic;
  read(n);
  var T = cpp_construct(2, Tree(n));
  var del = cpp_construct(2, vector((n - 1), false));
  {
    var t = 0;
    while ((t < (2)))
    {
      {
        var i = 0;
        while ((i < ((n - 1))))
        {
          var a: dynamic;
          read(a);
          a -= 1;
          T[t].add_edge((i + 1), a);
          i += 1;
        }
      }
      var time = 0;
      T[t].dfs(time);
      t += 1;
    }
  }
  {
    var t = 0;
    while ((t < (2)))
    {
      {
        var i = 0;
        while ((i < ((n - 1))))
        {
          var p = T[(1 - t)].edg[i];
          T[t].insert(p.first, p.second, i);
          i += 1;
        }
      }
      T[t].S.init();
      t += 1;
    }
  }
  var t = 0;
  var id: dynamic;
  var cur: dynamic;
  read(id);
  id -= 1;
  cur.push_back(id);
  del[1][id] = true;
  while ((!cur.empty()))
  {
    var prv = cur;
    cur.clear();
    if (((t % 2) == 0))
    {
      write("Blue", "\n");
    } else
    {
      write("Red", "\n");
    }
    for (var j in prv)
    {
      write((j + 1), " ");
    }
    write("\n");
    cur.clear();
    for (var j in prv)
    {
      var me = (T[t].get(j));
      for (var k in me)
      {
        if ((!del[t][k]))
        {
          del[t][k] = true;
          cur.push_back(k);
        }
      }
    }
    sort((cur).begin(), (cur).end());
    t = (1 - t);
  }
  return 0;
}
