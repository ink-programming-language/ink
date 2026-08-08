// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var fs = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var w: dynamic;

var h: dynamic;

var dw = [1, 0, -1, 0];

var dh = [0, 1, 0, -1];

var eps = 1e-8;

var visited = cpp_array(10, 61);

var vc: dynamic;

class st
{
  var ok: dynamic;
  var wei: dynamic;
  var g: dynamic;
}

func dfs(hh: dynamic, ww: dynamic, id: dynamic)
{
  visited[hh][ww] = true;
  var ret: dynamic;
  ret.push_back(P(hh, ww));
  rep(i, 4);
  {
    var nh = (hh + dh[i]);
    var nw = (ww + dw[i]);
    if (((((((0 <= nw) && (nw < w)) && (0 <= nh)) && (nh < h)) && (!visited[nh][nw])) && (vc[nh][nw] == (id + cpp_char("0")))))
    {
      var pl = dfs(nh, nw, id);
      ret.insert(ret.end(), pl.begin(), pl.end());
    }
  }
  return ret;
}

func stable(b: dynamic, l: dynamic, r: dynamic)
{
  var al = 0;
  var w = 4;
  rep(i, 4) += b[i].sc;
  rep(i, 4);
  {
    if (((isdigit(vc[(b[i].fs + 1)][b[i].sc]) && (vc[(b[i].fs + 1)][b[i].sc] != vc[b[i].fs][b[i].sc])) && (!visited[(b[i].fs + 1)][b[i].sc])))
    {
      var id = (vc[(b[i].fs + 1)][b[i].sc] - cpp_char("0"));
      var nb = dfs((b[i].fs + 1), b[i].sc, id);
      var l = 100;
      var r = -100;
      rep(j, 4);
      {
        var c = vc[(nb[j].fs - 1)][nb[j].sc];
        if ((isdigit(c) && (c != (id + cpp_char("0")))))
        {
          r = max(r, (0.5 + nb[j].sc));
          l = min(l, (-0.5 + nb[j].sc));
        }
      }
      var s = stable(nb, l, r);
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

func main()
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
    var l = 100;
    var r = -100;
    rep(j, w);
    if (isdigit(vc[0][j]))
    {
      l = min(l, j);
      r = max(r, j);
    }
    rep(i, h);
    rep(j, w)[i][j] = 0;
    var f = dfs(0, l, (vc[0][l] - cpp_char("0")));
    rep(i, h);
    rep(j, w)[i][j] = 0;
    write((if (stable(f, (-0.5 + l), (0.5 + r)).ok) "" else "UN"), "STABLE\n");
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var s: dynamic;
      read(s);
      vc.push_back(s);
    }
