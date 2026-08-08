// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

class Node
{
}

class Fun
{
}

var h: dynamic;

var w: dynamic;

var cr: dynamic;

var cc: dynamic;

var scr = cpp_array(512, 512);

var lnk: dynamic;

var evt: dynamic;

var scrp: dynamic;

var dmls: dynamic;

var funs: dynamic;

var act: dynamic;

func newline()
{
  cr += 1;
  cc = 0;
}

func draw(s: dynamic, hl: dynamic, fn: dynamic)
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
  var tag: dynamic;
  var text: dynamic;
  var cs: dynamic;
  var visible: dynamic;
  func Node(tag: dynamic)
  {
      this->tag = cpp_construct(tag);
      this->visible = cpp_construct(true);
    }
  func dump()
  {
      dump(0);
    }
  func dump(dep: dynamic)
  {
      (rep(cpp_name, dep) << cpp_char(" "));
      write(cpp_char("<"), tag, cpp_char(">"));
      write(" visi = ", visible);
      write(" text = ", text, "\n");
      rep(i, cs.size())[i]->dump((dep + 2));
      (rep(cpp_name, dep) << cpp_char(" "));
      write("</", tag, cpp_char(">"), "\n");
    }
  func render()
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
  func init()
  {
      visible = true;
      if ((tag == "script"))
      {
        assert(((cs.size() == 1) && (cs[0]->tag == "$text")));
        var file = cs[0]->text;
        var fs = scrp[file];
        rep(i, fs.size());
        {
          funs[fs[i].first] = fs[i].second;
        }
      }
      rep(i, cs.size())[i]->init();
    }
  func apply(vs: dynamic, k: dynamic, visi: dynamic)
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
  var asn: dynamic;
  func exec()
  {
      rep(i, asn.size());
      {
        act->apply(asn[i].first, 0, asn[i].second);
      }
    }
}

func lex_dml(s: dynamic)
{
  var ts: dynamic;
  var pos = 0;
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

func isbegin(t: dynamic)
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
    var i = 1;
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

func isend(t: dynamic)
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
    var i = 2;
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

func parse_dml(ts: dynamic)
{
  var root = cpp_new("$root");
  var stk: dynamic;
  stk.push_back(root);
  rep(i, ts.size());
  {
    if ((ts[i].size() == 0))
    {
      continue;
    }
    if (isbegin(ts[i]))
    {
      var node = cpp_new(ts[i].substr(1, (ts[i].size() - 2)));
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
      var node = cpp_new("$text");
      node->text = ts[i];
      stk.back()->cs.push_back(node);
    }
  }
  assert((stk.size() == 1));
  return root;
}

func parse_prop(s: dynamic)
{
  var ps: dynamic;
  var pos = 0;
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

var s: dynamic;

var ix: dynamic;

func parse_expr()
{
  var pos = ix;
  var props: dynamic;
  var rev: dynamic;
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
  var val = s.substr(pos, (ix - pos));
  var cur = (val == "true");
  var rs: dynamic;
  {
    var i = (cpp_cast(props.size()) - 1);
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

func parse_fun()
{
  var fun = cpp_new();
  var st = ix;
  while ((s[ix] != cpp_char("{")))
  {
    ix += 1;
  }
  var id = s.substr(st, (ix - st));
  ix += 1;
  while ((s[ix] != cpp_char("}")))
  {
    rep(i, es.size())->asn.push_back(es[i]);
  }
  ix += 1;
  return make_pair(id, fun);
}

func parse_ds(s: dynamic)
{
  s = s;
  ix = 0;
  var fs: dynamic;
  while ((ix < s.size()))
  {
    fs.push_back(parse_fun());
  }
  return fs;
}

func render(file: dynamic)
{
  cpp_statement("rep (i, h)");
  cr = cpp_assign(cc, "=", 0);
  file->render();
  act = file;
}

func click(x: dynamic, y: dynamic)
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

func getl(s: dynamic)
{
  getline(cin, s);
  assert(((s.size() == 0) || (s[(s.size() - 1)] != cpp_char("\r"))));
}

func main()
{
  var s: dynamic;
  getl(s);
  var n = atoi(s.c_str());
  getl(s);
  var m = atoi(s.c_str());
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    scr[i][j] = cpp_char(".");
    lnk[i][j] = 0;
    evt[i][j] = 0;
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    getl(s);
    if ((s.substr((s.size() - 4)) == ".dml"))
    {
      var file = s.substr(0, (s.size() - 4));
      getl(s);
      var ts = lex_dml(s);
      var root = parse_dml(ts);
      dmls[file] = root;
    } else if ((s.substr((s.size() - 3)) == ".ds"))
    {
      var file = s.substr(0, (s.size() - 3));
      getl(s);
      var fs = parse_ds(s);
      scrp[file] = fs;
    } else
    {
      assert(false);
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      var x: dynamic;
      var y: dynamic;
      getl(s);
      sscanf(s.c_str(), "%d%d", (&x), (&y));
      click(x, y);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("rep (j, w)");
      putchar(scr[i][j]);
      putchar(cpp_char("\n"));
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var K: dynamic;
    var buf = cpp_array(32);
    getl(s);
    sscanf(s.c_str(), "%d %d %d %s", (&w), (&h), (&K), buf);
    dmls[buf]->init();
    render(dmls[buf]);
  }
