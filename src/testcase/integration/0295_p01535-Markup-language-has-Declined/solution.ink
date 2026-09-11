// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

class Node
{
}

class Fun
{
}

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var cr: dynamic = cpp_uninitialized();

var cc: dynamic = cpp_uninitialized();

var scr: dynamic = cpp_array(512, 512);

var lnk: dynamic = cpp_uninitialized();

var evt: dynamic = cpp_uninitialized();

var scrp: dynamic = cpp_uninitialized();

var dmls: dynamic = cpp_uninitialized();

var funs: dynamic = cpp_uninitialized();

var act: dynamic = cpp_uninitialized();

func newline() -> dynamic
{
  cr += 1;
  cc = 0;
}

func draw(s: dynamic, hl: dynamic, fn: dynamic) -> dynamic
{
  rep(i, s.size());
  {
    if (((((0 <= cr) && (cr < h)) && (0 <= cc)) && (cc < w)))
    {
      scr[cr][cc] = s[i];
      lnk[cr][cc] = hl;
      evt[cr][cc] = fn;
      cc += 1;
    }
    if ((cc == w))
    {
      newline();
    }
  }
}

class Node
{
  var tag: dynamic = cpp_uninitialized();
  var text: dynamic = cpp_uninitialized();
  var cs: dynamic = cpp_uninitialized();
  var visible: dynamic = cpp_uninitialized();
  func Node(tag: dynamic) -> dynamic
  {
      self->tag = cpp_construct(tag);
      self->visible = cpp_construct(true);
    }
  func dump() -> dynamic
  {
      dump(0);
    }
  func dump(dep: dynamic) -> dynamic
  {
      (rep(cpp_name, dep) << cpp_char(" "));
      write(cpp_char("<"), tag, cpp_char(">"));
      write(" visi = ", visible);
      write(" text = ", text, "\n");
      rep(i, cs.size())[i]->dump((dep + 2));
      (rep(cpp_name, dep) << cpp_char(" "));
      write("</", tag, cpp_char(">"), "\n");
    }
  func render() -> dynamic
  {
      if ((!visible))
      {
        return;
      }
      if ((tag == "$text"))
      {
        draw(text, 0, 0);
      } else if ((tag == "script"))
      {
      } else if ((tag == "link"))
      {
        assert(((cs.size() == 1) && (cs[0]->tag == "$text")));
        assert(dmls[cs[0]->text]);
        draw(cs[0]->text, dmls[cs[0]->text], 0);
      } else if ((tag == "button"))
      {
        assert(((cs.size() == 1) && (cs[0]->tag == "$text")));
        draw(cs[0]->text, 0, funs[cs[0]->text]);
      } else if ((tag == "br"))
      {
        newline();
      } else
      {
        rep(i, cs.size())[i]->render();
      }
    }
  func init() -> dynamic
  {
      visible = true;
      if ((tag == "script"))
      {
        assert(((cs.size() == 1) && (cs[0]->tag == "$text")));
        var file: dynamic = cs[0]->text;
        var fs: dynamic = scrp[file];
        rep(i, fs.size());
        {
          funs[fs[i].first] = fs[i].second;
        }
      }
      rep(i, cs.size())[i]->init();
    }
  func apply(vs: dynamic, k: dynamic, visi: dynamic) -> dynamic
  {
      if ((vs[k] == tag))
      {
        if ((k == (cpp_cast(vs.size()) - 1)))
        {
          visible = visi;
        } else
        {
          k += 1;
        }
      }
      rep(i, cs.size())[i]->apply(vs, k, visi);
    }
}

class Fun
{
  var asn: dynamic = cpp_uninitialized();
  func exec() -> dynamic
  {
      rep(i, asn.size());
      {
        act->apply(asn[i].first, 0, asn[i].second);
      }
    }
}

func lex_dml(s: dynamic) -> dynamic
{
  var ts: dynamic = cpp_uninitialized();
  var pos: dynamic = 0;
  rep(i, s.size());
  {
    if ((s[i] == cpp_char("<")))
    {
      ts.push_back(s.substr(pos, (i - pos)));
      pos = i;
    } else if ((s[i] == cpp_char(">")))
    {
      ts.push_back(s.substr(pos, ((i + 1) - pos)));
      pos = (i + 1);
    }
  }
  ts.push_back(s.substr(pos));
  return ts;
}

func isbegin(t: dynamic) -> dynamic
{
  if ((t.size() < 3))
  {
    return false;
  }
  if (((t[0] != cpp_char("<")) || (t[(t.size() - 1)] != cpp_char(">"))))
  {
    return false;
  }
  {
    var i: dynamic = 1;
    while ((i < (cpp_cast(t.size()) - 1)))
    {
      assert((t[i] != cpp_char(" ")));
      if (((!islower(t[i])) && (!isupper(t[i]))))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func isend(t: dynamic) -> dynamic
{
  if ((t.size() < 4))
  {
    return false;
  }
  if ((((t[0] != cpp_char("<")) || (t[1] != cpp_char("/"))) || (t[(t.size() - 1)] != cpp_char(">"))))
  {
    return false;
  }
  {
    var i: dynamic = 2;
    while ((i < (cpp_cast(t.size()) - 1)))
    {
      assert((t[i] != cpp_char(" ")));
      if (((!islower(t[i])) && (!isupper(t[i]))))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func parse_dml(ts: dynamic) -> dynamic
{
  var root: dynamic = cpp_new("$root");
  var stk: dynamic = cpp_uninitialized();
  stk.push_back(root);
  rep(i, ts.size());
  {
    if ((ts[i].size() == 0))
    {
      continue;
    }
    if (isbegin(ts[i]))
    {
      var node: dynamic = cpp_new(ts[i].substr(1, (ts[i].size() - 2)));
      stk.back()->cs.push_back(node);
      if ((ts[i] != "<br>"))
      {
        stk.push_back(node);
      }
    } else if (isend(ts[i]))
    {
      assert((stk.back()->tag == ts[i].substr(2, (ts[i].size() - 3))));
      stk.pop_back();
    } else
    {
      var node: dynamic = cpp_new("$text");
      node->text = ts[i];
      stk.back()->cs.push_back(node);
    }
  }
  assert((stk.size() == 1));
  return root;
}

func parse_prop(s: dynamic) -> dynamic
{
  var ps: dynamic = cpp_uninitialized();
  var pos: dynamic = 0;
  rep(i, s.size());
  {
    if ((s[i] == cpp_char(".")))
    {
      ps.push_back(s.substr(pos, (i - pos)));
      pos = (i + 1);
    }
  }
  assert((s.substr(pos) == "visible"));
  return ps;
}

var s: dynamic = cpp_uninitialized();

var ix: dynamic = cpp_uninitialized();

func parse_expr() -> dynamic
{
  var pos: dynamic = ix;
  var props: dynamic = cpp_uninitialized();
  var rev: dynamic = cpp_uninitialized();
  while ((s[ix] != cpp_char(";")))
  {
    if (((s[ix] == cpp_char("!")) || (s[ix] == cpp_char("="))))
    {
      props.push_back(s.substr(pos, (ix - pos)));
      if ((s[ix] == cpp_char("!")))
      {
        ix += 2;
        pos = ix;
        rev.push_back(true);
      } else
      {
        ix += 1;
        pos = ix;
        rev.push_back(false);
      }
    } else
    {
      ix += 1;
    }
  }
  var val: dynamic = s.substr(pos, (ix - pos));
  var cur: dynamic = (val == "true");
  var rs: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (cpp_cast(props.size()) - 1);
    while ((i >= 0))
    {
      if (rev[i])
      {
        cur = (!cur);
      }
      rs.push_back(make_pair(parse_prop(props[i]), cur));
      i -= 1;
    }
  }
  ix += 1;
  return rs;
}

func parse_fun() -> dynamic
{
  var fun: dynamic = cpp_new();
  var st: dynamic = ix;
  while ((s[ix] != cpp_char("{")))
  {
    ix += 1;
  }
  var id: dynamic = s.substr(st, (ix - st));
  ix += 1;
  while ((s[ix] != cpp_char("}")))
  {
    rep(i, es.size())->asn.push_back(es[i]);
  }
  ix += 1;
  return make_pair(id, fun);
}

func parse_ds(s: dynamic) -> dynamic
{
  s = s;
  ix = 0;
  var fs: dynamic = cpp_uninitialized();
  while ((ix < s.size()))
  {
    fs.push_back(parse_fun());
  }
  return fs;
}

func render(file: dynamic) -> dynamic
{
  cpp_statement("rep (i, h)");
  cr = cpp_assign(cc, "=", 0);
  file->render();
  act = file;
}

func click(x: dynamic, y: dynamic) -> dynamic
{
  if (lnk[y][x])
  {
    funs.clear();
    lnk[y][x]->init();
    render(lnk[y][x]);
  } else if (evt[y][x])
  {
    evt[y][x]->exec();
    render(act);
  }
}

func getl(s: dynamic) -> dynamic
{
  getline(cin, s);
  assert(((s.size() == 0) || (s[(s.size() - 1)] != cpp_char("\r"))));
}

func main() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  getl(s);
  var n: dynamic = atoi(s.c_str());
  getl(s);
  var m: dynamic = atoi(s.c_str());
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    scr[i][j] = cpp_char(".");
    lnk[i][j] = 0;
    evt[i][j] = 0;
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    getl(s);
    if ((s.substr((s.size() - 4)) == ".dml"))
    {
      var file: dynamic = s.substr(0, (s.size() - 4));
      getl(s);
      var ts: dynamic = lex_dml(s);
      var root: dynamic = parse_dml(ts);
      dmls[file] = root;
    } else if ((s.substr((s.size() - 3)) == ".ds"))
    {
      var file: dynamic = s.substr(0, (s.size() - 3));
      getl(s);
      var fs: dynamic = parse_ds(s);
      scrp[file] = fs;
    } else
    {
      assert(false);
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      getl(s);
      sscanf(s.c_str(), "%d%d", (&x), (&y));
      click(x, y);
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      cpp_statement("rep (j, w)");
      putchar(scr[i][j]);
      putchar(cpp_char("\n"));
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var K: dynamic = cpp_uninitialized();
    var buf: dynamic = cpp_array(32);
    getl(s);
    sscanf(s.c_str(), "%d %d %d %s", (&w), (&h), (&K), buf);
    dmls[buf]->init();
    render(dmls[buf]);
  }
