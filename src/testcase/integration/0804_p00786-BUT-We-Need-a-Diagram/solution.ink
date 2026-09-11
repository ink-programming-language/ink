// Translated from solution.cpp.

func REP(i: dynamic, b: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=b;i<n;i++)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include<i");
}

var pb: dynamic = cpp_expression("#include<");

class Tree
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  func Tree() -> dynamic
  {
      self->l = cpp_construct(null);
      self->r = cpp_construct(null);
      self->d = cpp_construct(0);
    }
}

var parse: dynamic = cpp_uninitialized();

var R: dynamic = 200;

var C: dynamic = 500;

var m: dynamic = cpp_array(C, R);

func get_space(dl: dynamic, l: dynamic, dr: dynamic, r: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < min(dr, dl)))
    {
      ret = max(ret, ((l[i] + r[i]) + 2));
      if (((i + 1) != dl))
      {
        ret = max(ret, ((l[(i + 1)] + r[i]) + 1));
      }
      if (((i + 1) != dr))
      {
        ret = max(ret, ((l[i] + r[(i + 1)]) + 1));
      }
      i += 1;
    }
  }
  return ret;
}

func merge(dl: dynamic, l: dynamic, dr: dynamic, r: dynamic, dist: dynamic, opdist: dynamic) -> dynamic
{
  var i: dynamic = 0;
  {
    i = 0;
    while ((i < dl))
    {
      l[i] += dist;
      i += 1;
    }
  }
  {
    while ((i < dr))
    {
      l[i] = (r[i] - opdist);
      i += 1;
    }
  }
}

func make_pos(now: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  var height: dynamic = 0;
  l[0] = 0;
  r[0] = 0;
  if ((now->l == null))
  {
    height = 0;
  } else if ((now->r == null))
  {
    height = make_pos(now->l, (l + 1), (r + 1));
  } else
  {
    var tl: dynamic = cpp_array(R);
    var tr: dynamic = cpp_array(R);
    var dl: dynamic = make_pos(now->l, (l + 1), tr);
    var dr: dynamic = make_pos(now->r, tl, (r + 1));
    now->d = get_space(dr, tl, dl, tr);
    var depth: dynamic = max(dl, dr);
    merge(dl, (l + 1), depth, tl, (((now->d + 0)) / 2), (((now->d + 1)) / 2));
    merge(dr, (r + 1), depth, tr, (((now->d + 1)) / 2), (((now->d + 0)) / 2));
    height = depth;
  }
  return (height + 1);
}

func put_tree(depth: dynamic, x: dynamic, now: dynamic) -> dynamic
{
  m[depth][x] = now->c;
  if ((now->l == null))
  {
  } else if ((now->r == null))
  {
    m[(depth + 1)][x] = cpp_char("-");
    put_tree((depth + 2), x, now->l);
  } else
  {
    {
      var i: dynamic = (x - (now->d / 2));
      while ((i <= (x + (((now->d + 1)) / 2))))
      {
        m[(depth + 1)][i] = cpp_char("-");
        i += 1;
      }
    }
    put_tree((depth + 2), (x - (now->d / 2)), now->l);
    put_tree((depth + 2), (x + (((now->d + 1)) / 2)), now->r);
  }
}

func main() -> dynamic
{
  var tc: dynamic = 1;
  while (true)
  {
    var in_cpp: dynamic = "";
    var isend: dynamic = getdata(in_cpp);
    fill((&m[0][0]), (&m[R][0]), cpp_char(" "));
    var p: dynamic = 0;
    var root: dynamic = parse(in_cpp, p);
    var tl: dynamic = cpp_array(R);
    var tr: dynamic = cpp_array(R);
    rep(i, R)[i] = cpp_assign(tr[i], "=", -1);
    write(cpp_update(tc, "++"), ":", "\n");
    var tmp: dynamic = make_pos(root, tl, tr);
    var findwid: dynamic = 0;
    put_tree(0, (findwid + 1), root);
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var u: dynamic = cpp_uninitialized();
    var d: dynamic = cpp_uninitialized();
    {
      d = 0;
      while ((d < R))
      {
        var flag: dynamic = false;
        rep(j, C);
        if ((m[d][j] != cpp_char(" ")))
        {
          flag = true;
          break;
        }
        if (flag)
        {
          break;
        }
        d += 1;
      }
    }
    {
      u = (R - 1);
      while ((u >= 0))
      {
        var flag: dynamic = false;
        rep(j, C);
        if ((m[u][j] != cpp_char(" ")))
        {
          flag = true;
          break;
        }
        if (flag)
        {
          u += 1;
          break;
        }
        u -= 1;
      }
    }
    {
      l = 0;
      while ((l < C))
      {
        var flag: dynamic = false;
        REP(i, d, u);
        if ((m[i][l] != cpp_char(" ")))
        {
          flag = true;
          break;
        }
        if (flag)
        {
          break;
        }
        l += 1;
      }
    }
    {
      r = (C - 1);
      while ((r >= 0))
      {
        var flag: dynamic = false;
        REP(i, d, u);
        if ((m[i][r] != cpp_char(" ")))
        {
          flag = true;
          break;
        }
        if (flag)
        {
          r += 1;
          break;
        }
        r -= 1;
      }
    }
    {
      var i: dynamic = (u - 1);
      while ((i >= d))
      {
        r = 0;
        rep(j, C);
        if ((m[i][j] != cpp_char(" ")))
        {
          r = (j + 1);
        }
        (REP(j, l, r) << m[i][j]);
        write("\n");
        i -= 1;
      }
    }
    destoroy(root);
    if (isend)
    {
      break;
    }
  }
}

func parse(in_cpp: dynamic, p: dynamic) -> dynamic
{
  var now: dynamic = cpp_uninitialized();
  now = cpp_new();
  now->c = in_cpp[p];
  p += 1;
  if (((p < in_cpp.size()) && (in_cpp[p] != cpp_char("("))))
  {
  } else
  {
    p += 1;
    now->l = parse(in_cpp, p);
    if ((in_cpp[p] == cpp_char(")")))
    {
    } else if ((in_cpp[p] == cpp_char(",")))
    {
      p += 1;
      var tmp: dynamic = 0;
      now->r = parse(in_cpp, p);
    }
    p += 1;
  }
  return now;
}

func destoroy(in_cpp: dynamic) -> dynamic
{
  if ((in_cpp->l != null))
  {
    destoroy(in_cpp->l);
  }
  if ((in_cpp->r != null))
  {
    destoroy(in_cpp->r);
  }
  free(in_cpp);
}

func getdata(in_cpp: dynamic) -> dynamic
{
  var isend: dynamic = false;
  while (true)
  {
    var c: dynamic = getchar();
    if ((c == cpp_char(";")))
    {
      break;
    }
    if ((c == cpp_char(".")))
    {
      isend = true;
      break;
    }
    if ((((isalpha(c) || (c == cpp_char("("))) || (c == cpp_char(")"))) || (c == cpp_char(","))))
    {
      in_cpp += c;
    }
  }
  return isend;
}

func travarse(in_cpp: dynamic) -> dynamic
{
  write(in_cpp->c, " dist ", in_cpp->d);
  if ((in_cpp->l != null))
  {
    write(" l: ", in_cpp->l->c);
  }
  if ((in_cpp->r != null))
  {
    write(" r: ", in_cpp->r->c);
  }
  write("\n");
  if ((in_cpp->l != null))
  {
    travarse(in_cpp->l);
  }
  if ((in_cpp->r != null))
  {
    travarse(in_cpp->r);
  }
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      if ((tl[i] < 0))
      {
        findwid = max(findwid, (-tl[i]));
      } else
      {
        findwid = max(findwid, tl[i]);
      }
      if ((tr[i] < 0))
      {
        findwid = max(findwid, (-tr[i]));
      } else
      {
        findwid = max(findwid, tr[i]);
      }
    }
