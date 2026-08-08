// Translated from solution.cpp.

func chkmax(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chkmin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

func output(begin: dynamic, end: dynamic, out: dynamic = cerr)
{
  while ((begin != end))
  {
    ((out << ((*begin))) << " ");
    begin += 1;
  }
  (out << endl);
}

func output(x: dynamic, out: dynamic = cerr)
{
  output(x.begin(), x.end(), out);
}

func fast_io()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var G0: dynamic;

var G: dynamic;

var n: dynamic;

var m: dynamic;

var singleMut: dynamic;

var doubleMut: dynamic;

func newGene()
{
  singleMut.push_back(vector());
  doubleMut.push_back(vector());
  return cpp_update(G, "++");
}

var C: dynamic;

func read()
{
  read(G0, n, m);
  G = G0;
  singleMut.resize(G);
  doubleMut.resize(G);
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      var k: dynamic;
      read(a, k);
      if ((k == 1))
      {
        var b: dynamic;
        read(b);
        singleMut[a].push_back(b);
      } else
      {
        {
          var j = 0;
          while ((j < (k - 2)))
          {
            var a1 = newGene();
            var b: dynamic;
            read(b);
            doubleMut[a].emplace_back(b, a1);
            a = a1;
            j += 1;
          }
        }
        var b1: dynamic;
        var b2: dynamic;
        read(b1, b2);
        doubleMut[a].emplace_back(b1, b2);
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var k: dynamic;
      read(k);
      {
        var j = 0;
        while ((j < k))
        {
          read(cur[j]);
          j += 1;
        }
      }
      C.push_back(cur);
      i += 1;
    }
  }
}

var K = 2;

var mx = 55;

var maxG = 205;

var V: dynamic;

var go = cpp_array(K, mx);

var link = cpp_array(mx);

var term = cpp_array(mx);

func new_vertex()
{
  {
    var it = 0;
    while ((it < K))
    {
      go[V][it] = -1;
      it += 1;
    }
  }
  return (cpp_update(V, "++"));
}

func init()
{
  new_vertex();
}

func add_string(s: dynamic)
{
  var v = 0;
  for (var c in s)
  {
    if ((go[v][c] == -1))
    {
      go[v][c] = new_vertex();
    }
    v = go[v][c];
  }
  term[v] = 1;
}

func bfs()
{
  var q: dynamic;
  q.push(0);
  while ((!q.empty()))
  {
    var v = q.front();
    q.pop();
    if (term[link[v]])
    {
      term[v] = 1;
    }
    {
      var c = 0;
      while ((c < K))
      {
        if ((go[v][c] == -1))
        {
          go[v][c] = (if ((v == 0)) 0 else go[link[v]][c]);
        } else
        {
          q.push(go[v][c]);
          link[go[v][c]] = (if ((v == 0)) 0 else go[link[v]][c]);
        }
        c += 1;
      }
    }
  }
}

func build()
{
  init();
  for (var s in C)
  {
    add_string(s);
  }
  bfs();
}

var rev_single: dynamic;

var rev_double_l: dynamic;

var rev_double_r: dynamic;

func prepare_aux()
{
  rev_single.resize(G);
  rev_double_l.resize(G);
  rev_double_r.resize(G);
  {
    var i = 0;
    while ((i < G))
    {
      for (var v in singleMut[i])
      {
        rev_single[v].push_back(i);
      }
      for (var pp in doubleMut[i])
      {
        rev_double_l[pp.first].emplace_back(pp.second, i);
        rev_double_r[pp.second].emplace_back(pp.first, i);
      }
      i += 1;
    }
  }
}

class Achievement
{
  var t: dynamic;
  var geneId: dynamic;
  var from_cpp: dynamic;
  var to: dynamic;
}

func operator_less(a: dynamic, b: dynamic)
{
  return (a.t < b.t);
}

var INF = (cpp_cast(1) << 63);

var S: dynamic;

var dist = cpp_array(mx, mx, maxG);

var used = cpp_array(mx, mx, maxG);

func djkstra()
{
  {
    var i = 0;
    while ((i < G))
    {
      {
        var from_cpp = 0;
        while ((from_cpp < V))
        {
          {
            var to = 0;
            while ((to < V))
            {
              dist[i][from_cpp][to] = INF;
              to += 1;
            }
          }
          from_cpp += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < V))
    {
      {
        var it = 0;
        while ((it < K))
        {
          if (((!term[i]) && (!term[go[i][it]])))
          {
            S.insert([1, it, i, go[i][it]]);
            dist[it][i][go[i][it]] = 1;
          }
          it += 1;
        }
      }
      i += 1;
    }
  }
  while ((!S.empty()))
  {
    var cur = ((*S.begin()));
    S.erase(S.begin());
    var gene = cur.geneId;
    var from_cpp = cur.from_cpp;
    var to = cur.to;
    var t = cur.t;
    if (used[gene][from_cpp][to])
    {
      continue;
    }
    for (var v1 in rev_single[gene])
    {
      if (chkmin(dist[v1][from_cpp][to], t))
      {
        S.insert([dist[v1][from_cpp][to], v1, from_cpp, to]);
      }
    }
    for (var pp in rev_double_l[gene])
    {
      var v1 = pp.first;
      var v2 = pp.second;
      {
        var final_cpp = 0;
        while ((final_cpp < V))
        {
          if (((!term[final_cpp]) && chkmin(dist[v2][from_cpp][final_cpp], (t + dist[v1][to][final_cpp]))))
          {
            S.insert([dist[v2][from_cpp][final_cpp], v2, from_cpp, final_cpp]);
          }
          final_cpp += 1;
        }
      }
    }
    for (var pp in rev_double_r[gene])
    {
      var v1 = pp.first;
      var v2 = pp.second;
      {
        var start = 0;
        while ((start < V))
        {
          if (((!term[start]) && chkmin(dist[v2][start][to], (dist[v1][start][from_cpp] + t))))
          {
            S.insert([dist[v2][start][to], v2, start, to]);
          }
          start += 1;
        }
      }
    }
  }
}

func print_ans()
{
  {
    var i = 2;
    while ((i < G0))
    {
      var opt = INF;
      {
        var j = 0;
        while ((j < V))
        {
          if ((!term[j]))
          {
            chkmin(opt, dist[i][0][j]);
          }
          j += 1;
        }
      }
      if ((opt == INF))
      {
        write("YES\n");
      } else
      {
        write("NO ", opt, cpp_char("\n"));
      }
      i += 1;
    }
  }
}

func main()
{
  fast_io();
  read();
  build();
  prepare_aux();
  djkstra();
  print_ans();
}
