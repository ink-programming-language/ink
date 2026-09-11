// Translated from solution.cpp.

func chkmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chkmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

func output(begin: dynamic, end: dynamic, out: dynamic = cerr) -> dynamic
{
  while ((begin != end))
  {
    ((out << ((*begin))) << " ");
    begin += 1;
  }
  (out << endl);
}

func output(x: dynamic, out: dynamic = cerr) -> dynamic
{
  output(x.begin(), x.end(), out);
}

func fast_io() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var G0: dynamic = cpp_uninitialized();

var G: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var singleMut: dynamic = cpp_uninitialized();

var doubleMut: dynamic = cpp_uninitialized();

func newGene() -> dynamic
{
  singleMut.push_back(vector());
  doubleMut.push_back(vector());
  return cpp_update(G, "++");
}

var C: dynamic = cpp_uninitialized();

func read() -> dynamic
{
  read(G0, n, m);
  G = G0;
  singleMut.resize(G);
  doubleMut.resize(G);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      read(a, k);
      if ((k == 1))
      {
        var b: dynamic = cpp_uninitialized();
        read(b);
        singleMut[a].push_back(b);
      } else
      {
        {
          var j: dynamic = 0;
          while ((j < (k - 2)))
          {
            var a1: dynamic = newGene();
            var b: dynamic = cpp_uninitialized();
            read(b);
            doubleMut[a].emplace_back(b, a1);
            a = a1;
            j += 1;
          }
        }
        var b1: dynamic = cpp_uninitialized();
        var b2: dynamic = cpp_uninitialized();
        read(b1, b2);
        doubleMut[a].emplace_back(b1, b2);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var k: dynamic = cpp_uninitialized();
      read(k);
      {
        var j: dynamic = 0;
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

var K: dynamic = 2;

var mx: dynamic = 55;

var maxG: dynamic = 205;

var V: dynamic = cpp_uninitialized();

var go: dynamic = cpp_array(K, mx);

var link: dynamic = cpp_array(mx);

var term: dynamic = cpp_array(mx);

func new_vertex() -> dynamic
{
  {
    var it: dynamic = 0;
    while ((it < K))
    {
      go[V][it] = -1;
      it += 1;
    }
  }
  return (cpp_update(V, "++"));
}

func init() -> dynamic
{
  new_vertex();
}

func add_string(s: dynamic) -> dynamic
{
  var v: dynamic = 0;
  for (var c: dynamic in s)
  {
    if ((go[v][c] == -1))
    {
      go[v][c] = new_vertex();
    }
    v = go[v][c];
  }
  term[v] = 1;
}

func bfs() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  q.push(0);
  while ((!q.empty()))
  {
    var v: dynamic = q.front();
    q.pop();
    if (term[link[v]])
    {
      term[v] = 1;
    }
    {
      var c: dynamic = 0;
      while ((c < K))
      {
        if ((go[v][c] == -1))
        {
          go[v][c] = ( ((v == 0)) ? 0 : go[link[v]][c]);
        } else
        {
          q.push(go[v][c]);
          link[go[v][c]] = ( ((v == 0)) ? 0 : go[link[v]][c]);
        }
        c += 1;
      }
    }
  }
}

func build() -> dynamic
{
  init();
  for (var s: dynamic in C)
  {
    add_string(s);
  }
  bfs();
}

var rev_single: dynamic = cpp_uninitialized();

var rev_double_l: dynamic = cpp_uninitialized();

var rev_double_r: dynamic = cpp_uninitialized();

func prepare_aux() -> dynamic
{
  rev_single.resize(G);
  rev_double_l.resize(G);
  rev_double_r.resize(G);
  {
    var i: dynamic = 0;
    while ((i < G))
    {
      for (var v: dynamic in singleMut[i])
      {
        rev_single[v].push_back(i);
      }
      for (var pp: dynamic in doubleMut[i])
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
  var t: dynamic = cpp_uninitialized();
  var geneId: dynamic = cpp_uninitialized();
  var from_cpp: dynamic = cpp_uninitialized();
  var to: dynamic = cpp_uninitialized();
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return (a.t < b.t);
}

var INF: dynamic = (cpp_cast(1) << 63);

var S: dynamic = cpp_uninitialized();

var dist: dynamic = cpp_array(mx, mx, maxG);

var used: dynamic = cpp_array(mx, mx, maxG);

func djkstra() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < G))
    {
      {
        var from_cpp: dynamic = 0;
        while ((from_cpp < V))
        {
          {
            var to: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < V))
    {
      {
        var it: dynamic = 0;
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
    var cur: dynamic = ((*S.begin()));
    S.erase(S.begin());
    var gene: dynamic = cur.geneId;
    var from_cpp: dynamic = cur.from_cpp;
    var to: dynamic = cur.to;
    var t: dynamic = cur.t;
    if (used[gene][from_cpp][to])
    {
      continue;
    }
    for (var v1: dynamic in rev_single[gene])
    {
      if (chkmin(dist[v1][from_cpp][to], t))
      {
        S.insert([dist[v1][from_cpp][to], v1, from_cpp, to]);
      }
    }
    for (var pp: dynamic in rev_double_l[gene])
    {
      var v1: dynamic = pp.first;
      var v2: dynamic = pp.second;
      {
        var final_cpp: dynamic = 0;
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
    for (var pp: dynamic in rev_double_r[gene])
    {
      var v1: dynamic = pp.first;
      var v2: dynamic = pp.second;
      {
        var start: dynamic = 0;
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

func print_ans() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < G0))
    {
      var opt: dynamic = INF;
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  fast_io();
  read();
  build();
  prepare_aux();
  djkstra();
  print_ans();
}
