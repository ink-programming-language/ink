// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var fs: dynamic = cpp_expression("#incl");

var sc: dynamic = cpp_expression("#inclu");

var w: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var dw: dynamic = [1, 0, -1, 0];

var dh: dynamic = [0, 1, 0, -1];

var eps: dynamic = 1e-8;

var visited: dynamic = cpp_array(10, 61);

var vc: dynamic = cpp_uninitialized();

class st
{
  var ok: dynamic = cpp_uninitialized();
  var wei: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
}

func dfs(hh: dynamic, ww: dynamic, id: dynamic) -> dynamic
{
  visited[hh][ww] = true;
  var ret: dynamic = cpp_uninitialized();
  ret.push_back(P(hh, ww));
  rep(i, 4);
  {
    var nh: dynamic = (hh + dh[i]);
    var nw: dynamic = (ww + dw[i]);
    if (((((((0 <= nw) && (nw < w)) && (0 <= nh)) && (nh < h)) && (!visited[nh][nw])) && (vc[nh][nw] == (id + cpp_char("0")))))
    {
      var pl: dynamic = dfs(nh, nw, id);
      ret.insert(ret.end(), pl.begin(), pl.end());
    }
  }
  return ret;
}

func stable(b: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  var al: dynamic = 0;
  var w: dynamic = 4;
  rep(i, 4) += b[i].sc;
  rep(i, 4);
  {
    if (((isdigit(vc[(b[i].fs + 1)][b[i].sc]) && (vc[(b[i].fs + 1)][b[i].sc] != vc[b[i].fs][b[i].sc])) && (!visited[(b[i].fs + 1)][b[i].sc])))
    {
      var id: dynamic = (vc[(b[i].fs + 1)][b[i].sc] - cpp_char("0"));
      var nb: dynamic = dfs((b[i].fs + 1), b[i].sc, id);
      var l: dynamic = 100;
      var r: dynamic = -100;
      rep(j, 4);
      {
        var c: dynamic = vc[(nb[j].fs - 1)][nb[j].sc];
        if ((isdigit(c) && (c != (id + cpp_char("0")))))
        {
          r = max(r, (0.5 + nb[j].sc));
          l = min(l, (-0.5 + nb[j].sc));
        }
      }
      var s: dynamic = stable(nb, l, r);
      if ((!s.ok))
      {
        return [false, 0, 0];
      }
      al += (s.wei * s.g);
      w += s.wei;
    }
  }
  if ((((l + eps) < (al / w)) && (((al / w) + eps) < r)))
  {
    return [true, w, (al / w)];
  } else
  {
    return [false, 0, 0];
  }
}

func main() -> dynamic
{
  while (true)
  {
    read(w, h);
    if ((!w))
    {
      break;
    }
    vc.clear();
    vc.push_back(string_cpp(w, cpp_char(".")));
    reverse(vc.begin(), vc.end());
    var l: dynamic = 100;
    var r: dynamic = -100;
    rep(j, w);
    if (isdigit(vc[0][j]))
    {
      l = min(l, j);
      r = max(r, j);
    }
    rep(i, h);
    rep(j, w)[i][j] = 0;
    var f: dynamic = dfs(0, l, (vc[0][l] - cpp_char("0")));
    rep(i, h);
    rep(j, w)[i][j] = 0;
    write(( (stable(f, (-0.5 + l), (0.5 + r)).ok) ? "" : "UN"), "STABLE\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var s: dynamic = cpp_uninitialized();
      read(s);
      vc.push_back(s);
    }
