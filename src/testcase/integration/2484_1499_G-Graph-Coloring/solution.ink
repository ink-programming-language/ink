// Translated from solution.cpp.

func range(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (n); ++i)");
}

func all(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func rall(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var ar = cpp_expression("#incl");

var INFi = (2e9 + 5);

var maxN = (4e5 + 100);

var md = 998244353;

var INF = 2e18;

func add(a: dynamic, b: dynamic)
{
  return if (((a + b) >= md)) ((a + b) - md) else (a + b);
}

func sub(a: dynamic, b: dynamic)
{
  return if (((a - b) < 0)) ((a - b) + md) else (a - b);
}

func mul(a: dynamic, b: dynamic)
{
  return ((((1 * a) * b)) % md);
}

var color = cpp_array(maxN);

var to = cpp_array(maxN);

var hashes = cpp_array(maxN);

var H = 0;

class path
{
  var vd: dynamic;
  var edges: dynamic;
  var i: dynamic;
  var start_col: dynamic;
  var end_col: dynamic;
  func add_edge(v: dynamic, u: dynamic, k: dynamic)
  {
      if (vd.empty())
      {
        vd.push_back(u);
        vd.push_back(v);
        to[v] = cpp_assign(to[u], "=", i);
        edges.push_back(k);
        H = add(H, hashes[k]);
        color[k] = 1;
        start_col = cpp_assign(end_col, "=", 1);
        return;
      }
      to[v] = -1;
      if ((vd.back() == v))
      {
        vd.push_back(u);
        edges.push_back(k);
        end_col ^= 1;
        if (end_col)
        {
          H = add(H, hashes[k]);
          color[k] = 1;
        } else
        {
          color[k] = -1;
        }
      } else if ((vd.front() == v))
      {
        vd.push_front(u);
        edges.push_front(k);
        start_col ^= 1;
        if (start_col)
        {
          H = add(H, hashes[k]);
          color[k] = 1;
        } else
        {
          color[k] = -1;
        }
      } else
      {
        assert(0);
      }
      if ((vd.back() == vd.front()))
      {
        to[vd.back()] = cpp_assign(to[vd.front()], "=", -1);
        return;
      }
      to[u] = i;
    }
  func merge(b: dynamic, v: dynamic, u: dynamic, k: dynamic)
  {
      if ((b.i == i))
      {
        add_edge(v, u, k);
        return;
      }
      {
        var j = 0;
        while ((j < b.edges.size()))
        {
          if ((((j ^ b.start_col)) & 1))
          {
            H = sub(H, hashes[b.edges[j]]);
          }
          j += 1;
        }
      }
      if ((b.vd.front() == u))
      {
        reverse(all(b.vd));
        reverse(all(b.edges));
      }
      to[b.vd.back()] = cpp_assign(to[b.vd.front()], "=", -1);
      to[v] = -1;
      b.edges.push_back(k);
      if ((vd.front() == v))
      {
        while ((!b.vd.empty()))
        {
          k = b.edges.back();
          start_col ^= 1;
          if (start_col)
          {
            H = add(H, hashes[k]);
            color[k] = 1;
          } else
          {
            color[k] = -1;
          }
          edges.push_front(k);
          vd.push_front(b.vd.back());
          b.vd.pop_back();
          b.edges.pop_back();
        }
      } else
      {
        assert((vd.back() == v));
        while ((!b.vd.empty()))
        {
          k = b.edges.back();
          edges.push_back(k);
          end_col ^= 1;
          if (end_col)
          {
            H = add(H, hashes[k]);
            color[k] = 1;
          } else
          {
            color[k] = -1;
          }
          vd.push_back(b.vd.back());
          b.vd.pop_back();
          b.edges.pop_back();
        }
      }
      assert((vd.back() != vd.front()));
      to[vd.back()] = cpp_assign(to[vd.front()], "=", i);
    }
}

var paths = cpp_array(maxN);

var R = 0;

func create()
{
  paths[R].i = R;
  return cpp_update(R, "++");
}

var K = 1;

func init()
{
  hashes[0] = 1;
  {
    var i = 1;
    while ((i < maxN))
    {
      hashes[i] = mul(hashes[(i - 1)], 2);
      i += 1;
    }
  }
}

func new_edge(u: dynamic, v: dynamic)
{
  if (((to[u] == -1) && (to[v] == -1)))
  {
    var nw = create();
    paths[nw].add_edge(v, u, K);
    K += 1;
    return;
  }
  if (((to[u] != -1) && (to[v] != -1)))
  {
    if ((paths[to[u]].vd.size() > paths[to[v]].vd.size()))
    {
      swap(u, v);
    }
    paths[to[v]].merge(paths[to[u]], v, u, K);
    K += 1;
    return;
  }
  if ((to[v] == -1))
  {
    swap(u, v);
  }
  paths[to[v]].add_edge(v, u, K);
  K += 1;
}

func print()
{
  var answer: dynamic;
  {
    var i = 1;
    while ((i <= K))
    {
      if ((color[i] == 1))
      {
        answer.push_back(i);
      }
      i += 1;
    }
  }
  write(answer.size(), cpp_char(" "));
  for (var u in answer)
  {
    write(u, cpp_char(" "));
  }
  write("\n");
}

func solve()
{
  init();
  var n1: dynamic;
  var n2: dynamic;
  var m: dynamic;
  read(n1, n2, m);
  range(i, ((n1 + n2) + 5))[i] = -1;
  var q: dynamic;
  read(q);
}

func main()
{
  var tests = 1;
  return 0;
}

func range(argument_0: dynamic, argument_1: dynamic)
{
    var u: dynamic;
    var v: dynamic;
    read(u, v);
    v += n1;
    new_edge(u, v);
  }

func range(argument_0: dynamic, argument_1: dynamic)
{
    var t: dynamic;
    read(t);
    if ((t == 1))
    {
      var v1: dynamic;
      var v2: dynamic;
      read(v1, v2);
      v2 += n1;
      new_edge(v1, v2);
      write(H, "\n");
    } else
    {
      print();
    }
  }

func range(argument_0: dynamic, argument_1: dynamic)
{
    solve();
  }
