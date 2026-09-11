// Translated from solution.cpp.

var sm: dynamic = cpp_uninitialized();

var str: dynamic = cpp_array(500005);

var nstr: dynamic = cpp_array(500005);

var graph: dynamic = cpp_array(500005);

var v: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var R: dynamic = cpp_array(500005);

var RC: dynamic = cpp_array(500005);

var LC: dynamic = cpp_array(500005);

var ele: dynamic = cpp_uninitialized();

var len: dynamic = cpp_array(500005);

var dag: dynamic = cpp_array(500005);

var stk: dynamic = cpp_uninitialized();

var tstk: dynamic = cpp_uninitialized();

var col: dynamic = cpp_array(500005);

var dis: dynamic = cpp_array(500005);

var low: dynamic = cpp_array(500005);

var id: dynamic = cpp_array(500005);

var elements: dynamic = cpp_array(500005);

var node: dynamic = cpp_uninitialized();

var edge: dynamic = cpp_uninitialized();

var components: dynamic = cpp_uninitialized();

var timer: dynamic = cpp_uninitialized();

func dfs(u: dynamic) -> dynamic
{
  var sz: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var tem: dynamic = cpp_uninitialized();
  col[u] = 1;
  timer += 1;
  dis[u] = cpp_assign(low[u], "=", timer);
  stk.push(u);
  sz = graph[u].size();
  {
    i = 0;
    while ((i < sz))
    {
      v = graph[u][i];
      if ((col[v] == 1))
      {
        low[u] = min(low[v], low[u]);
      } else if ((col[v] == 0))
      {
        dfs(v);
        low[u] = min(low[u], low[v]);
      }
      i += 1;
    }
  }
  if ((dis[u] == low[u]))
  {
    components += 1;
    while (true)
    {
      tem = stk.top();
      tstk.push(tem);
      stk.pop();
      id[tem] = components;
      if ((tem <= ele))
      {
        elements[components] += 1;
      }
      if (((RC[components] == 0) && (LC[components] == 0)))
      {
        RC[components] = R[tem];
        LC[components] = len[tem];
      } else
      {
        if ((RC[components] > R[tem]))
        {
          RC[components] = R[tem];
          LC[components] = len[tem];
        } else if ((RC[components] == R[tem]))
        {
          LC[components] = min(LC[components], len[tem]);
        }
      }
      col[tem] = 2;
      if (!(((tem != u))))
      {
        break;
      }
    }
    while ((!tstk.empty()))
    {
      tem = tstk.top();
      R[tem] = RC[components];
      len[tem] = LC[components];
      tstk.pop();
    }
  }
  return;
}

func scctarjan() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= node))
    {
      if ((col[i] == 0))
      {
        dfs(i);
      }
      i += 1;
    }
  }
  return;
}

func new_DAG_graph() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= node))
    {
      sz = graph[i].size();
      {
        j = 0;
        while ((j < sz))
        {
          u = id[i];
          v = id[graph[i][j]];
          if ((u != v))
          {
            dag[u].push_back(v);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return;
}

func dfs2(n: dynamic) -> dynamic
{
  var sz: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  col[n] = 1;
  sz = dag[n].size();
  {
    i = 0;
    while ((i < sz))
    {
      j = dag[n][i];
      if ((col[j] == 0))
      {
        dfs2(j);
        if ((RC[n] == RC[j]))
        {
          LC[n] = min(LC[n], LC[j]);
        } else if ((RC[n] > RC[j]))
        {
          RC[n] = RC[j];
          LC[n] = LC[j];
        }
      } else if (((col[j] == 1) || (col[j] == 2)))
      {
        if ((RC[n] == RC[j]))
        {
          LC[n] = min(LC[n], LC[j]);
        } else if ((RC[n] > RC[j]))
        {
          RC[n] = RC[j];
          LC[n] = LC[j];
        }
      }
      i += 1;
    }
  }
  col[n] = 2;
  return;
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var ansr: dynamic = cpp_uninitialized();
  var ansl: dynamic = cpp_uninitialized();
  c = 0;
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%s", str);
      l = strlen(str);
      r = 0;
      {
        j = 0;
        while ((j < l))
        {
          str[j] = tolower(str[j]);
          if ((str[j] == cpp_char("r")))
          {
            r += 1;
          }
          j += 1;
        }
      }
      if ((sm.find(str) == sm.end()))
      {
        c += 1;
        R[c] = r;
        len[c] = l;
        sm[str] = c;
      }
      v.push_back(sm[str]);
      i += 1;
    }
  }
  ele = c;
  read(m);
  {
    i = 1;
    while ((i <= m))
    {
      scanf("%s %s", str, nstr);
      l = strlen(str);
      r = 0;
      {
        j = 0;
        while ((j < l))
        {
          str[j] = tolower(str[j]);
          if ((str[j] == cpp_char("r")))
          {
            r += 1;
          }
          j += 1;
        }
      }
      if ((sm.find(str) == sm.end()))
      {
        c += 1;
        R[c] = r;
        len[c] = l;
        sm[str] = c;
      }
      l = strlen(nstr);
      r = 0;
      {
        j = 0;
        while ((j < l))
        {
          nstr[j] = tolower(nstr[j]);
          if ((nstr[j] == cpp_char("r")))
          {
            r += 1;
          }
          j += 1;
        }
      }
      if ((sm.find(nstr) == sm.end()))
      {
        c += 1;
        R[c] = r;
        len[c] = l;
        sm[nstr] = c;
      }
      graph[sm[str]].push_back(sm[nstr]);
      i += 1;
    }
  }
  node = c;
  scctarjan();
  new_DAG_graph();
  memset(col, 0, cpp_sizeof((col)));
  ansr = cpp_assign(ansl, "=", 0);
  sz = v.size();
  var tot: dynamic = 0;
  {
    i = 1;
    while ((i <= components))
    {
      tot += elements[i];
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= components))
    {
      if ((col[i] != 2))
      {
        dfs2(i);
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < sz))
    {
      ansr += (RC[id[v[i]]]);
      ansl += (LC[id[v[i]]]);
      i += 1;
    }
  }
  write(ansr, " ", ansl, "\n");
  return 0;
}
