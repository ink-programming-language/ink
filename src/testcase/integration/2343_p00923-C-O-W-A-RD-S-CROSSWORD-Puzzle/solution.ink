// Translated from solution.cpp.

func ALL(c: dynamic) -> dynamic
{
  return cpp_expression("// very kuso prob");
}

func IN(l: dynamic, v: dynamic, r: dynamic) -> dynamic
{
  return cpp_expression("// very kuso pr");
}

func UNIQUE(v: dynamic) -> dynamic
{
  sort(ALL(v));
  v.erase(unique(ALL(v)), v.end());
}

func DUMP(x: dynamic) -> dynamic
{
  return cpp_expression("// very kuso problem #in");
}

func LINE() -> dynamic
{
  return cpp_expression("// very kuso problem #include");
}

func range(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for(int i=(int)l;i<(int)(r);i++)");
}

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  return ((is >> p.first) >> p.second);
}

func operator_shift_right(is: dynamic, t: dynamic) -> dynamic
{
  return (is >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic) -> dynamic
{
  return ((is >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic) -> dynamic
{
  return (((is >> get(t)) >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, t: dynamic) -> dynamic
{
  return ((((is >> get(t)) >> get(t)) >> get(t)) >> get(t));
}

func operator_shift_right(is: dynamic, as_cpp: dynamic) -> dynamic
{
  range(i, 0, as_cpp.size());
  (is >> as_cpp[i]);
  return is;
}

func operator_shift_left(os: dynamic, ss: dynamic) -> dynamic
{
  for (var a: dynamic in ss)
  {
    if ((a != ss.begin()))
    {
      (os << " ");
    }
    (os << a);
  }
  return os;
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((os << p.first) << " ") << p.second);
}

func operator_shift_left(os: dynamic, m: dynamic) -> dynamic
{
  var isF: dynamic = true;
  for (var p: dynamic in m)
  {
    if ((!isF))
    {
      (os << endl);
    }
    (os << p);
    isF = false;
  }
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (os << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (((os << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (((((os << get(t)) << " ") << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, t: dynamic) -> dynamic
{
  return (((((((os << get(t)) << " ") << get(t)) << " ") << get(t)) << " ") << get(t));
}

func operator_shift_left(os: dynamic, as_cpp: dynamic) -> dynamic
{
  range(i, 0, as_cpp.size());
  {
    if ((i != 0))
    {
      (os << " ");
    }
    (os << as_cpp[i]);
  }
  return os;
}

func operator_shift_left(os: dynamic, as_cpp: dynamic) -> dynamic
{
  range(i, 0, as_cpp.size());
  {
    if ((i != 0))
    {
      (os << endl);
    }
    (os << as_cpp[i]);
  }
  return os;
}

class Edge
{
  var c: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
}

class NFA
{
  var ONE: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  var es: dynamic = cpp_uninitialized();
  var f_cache: dynamic = cpp_uninitialized();
  var t_cache: dynamic = cpp_uninitialized();
  var reachable: dynamic = cpp_uninitialized();
  var trans: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  func NFA(N: dynamic = 0) -> dynamic
  {
      self->N = cpp_construct(N);
      self->s = cpp_construct(0);
      self->t = cpp_construct(1);
      f_cache = cpp_assign(t_cache, "=", vector(N));
    }
  func add_edge(c: dynamic, f: dynamic, t: dynamic) -> dynamic
  {
      var eid: dynamic = es.size();
      es.push_back([c, f, t]);
      f_cache[f].push_back(eid);
      t_cache[t].push_back(eid);
    }
  func disjoint(a: dynamic, b: dynamic) -> dynamic
  {
      var res: dynamic = ( ((a->es.size() < b->es.size())) ? b : a);
      var add: dynamic = ( ((a->es.size() < b->es.size())) ? a : b);
      res->f_cache.resize((res->N + add->N));
      res->t_cache.resize((res->N + add->N));
      var eid: dynamic = res->es.size();
      for (var e: dynamic in add->es)
      {
        e.f += res->N;
        e.t += res->N;
        res->es.push_back(e);
        res->f_cache[e.f].push_back(eid);
        res->t_cache[e.t].push_back(eid);
        eid += 1;
      }
      res->N += add->N;
      return res;
    }
  func Union(a: dynamic, b: dynamic) -> dynamic
  {
      var bl: dynamic = (a->es.size() < b->es.size());
      var as_cpp: dynamic = (( (bl) ? b->N : 0) + a->s);
      var at: dynamic = (( (bl) ? b->N : 0) + a->t);
      var bs: dynamic = (( ((!bl)) ? a->N : 0) + b->s);
      var bt: dynamic = (( ((!bl)) ? a->N : 0) + b->t);
      var S: dynamic = (a->N + b->N);
      var T: dynamic = ((a->N + b->N) + 1);
      var ab: dynamic = disjoint(a, b);
      var ab1: dynamic = disjoint(ab, (&ONE));
      var res: dynamic = disjoint(ab1, (&ONE));
      res->add_edge(cpp_char("-"), S, as_cpp);
      res->add_edge(cpp_char("-"), S, bs);
      res->add_edge(cpp_char("-"), at, T);
      res->add_edge(cpp_char("-"), bt, T);
      res->s = S;
      res->t = T;
      return res;
    }
  func Concat(a: dynamic, b: dynamic) -> dynamic
  {
      var bl: dynamic = (a->es.size() < b->es.size());
      var as_cpp: dynamic = (( (bl) ? b->N : 0) + a->s);
      var at: dynamic = (( (bl) ? b->N : 0) + a->t);
      var bs: dynamic = (( ((!bl)) ? a->N : 0) + b->s);
      var bt: dynamic = (( ((!bl)) ? a->N : 0) + b->t);
      var res: dynamic = disjoint(a, b);
      res->add_edge(cpp_char("-"), at, bs);
      res->s = as_cpp;
      res->t = bt;
      return res;
    }
  func Star(a: dynamic) -> dynamic
  {
      var res: dynamic = disjoint(a, (&ONE));
      res->add_edge(cpp_char("-"), (res->N - 1), a->s);
      res->add_edge(cpp_char("-"), a->t, (res->N - 1));
      res->s = cpp_assign(res->t, "=", (res->N - 1));
      return res;
    }
  func create_reachable() -> dynamic
  {
      var que: dynamic = cpp_uninitialized();
      que.push([t, 0]);
      reachable = vector(N, vector(4));
      reachable[t][0] = true;
      while ((!que.empty()))
      {
        var t: dynamic = cpp_uninitialized();
        var d: dynamic = cpp_uninitialized();
        tie(t, d) = que.front();
        que.pop();
        for (var eid: dynamic in t_cache[t])
        {
          var e: dynamic = es[eid];
          if ((e.c == cpp_char("-")))
          {
            if ((!reachable[e.f][d]))
            {
              reachable[e.f][d] = true;
              que.push([e.f, d]);
            }
          } else
          {
            if ((((d + 1) < reachable[e.f].size()) && (!reachable[e.f][(d + 1)])))
            {
              reachable[e.f][(d + 1)] = true;
              que.push([e.f, (d + 1)]);
            }
          }
        }
      }
    }
  func create_trans() -> dynamic
  {
      trans = vector(N, vector(27));
      range(st, 0, N);
      {
        passed[st] = true;
        var que: dynamic = cpp_uninitialized();
        range(i, 0, passed.size());
        if (passed[i])
        {
          que.push(i);
        }
        while ((!que.empty()))
        {
          var s: dynamic = que.front();
          que.pop();
          for (var eid: dynamic in f_cache[s])
          {
            if (((es[eid].c == cpp_char("-")) && (!passed[es[eid].t])))
            {
              passed[es[eid].t] = true;
              que.push(es[eid].t);
            }
          }
        }
        range(i, 0, N);
        if (passed[i])
        {
          trans[st][26].push_back(i);
        }
      }
      range(st, 0, N);
      range(c, cpp_char("A"), (cpp_char("Z") + 1));
      {
        for (var s: dynamic in trans[st][26])
        {
          passed[s] = true;
        }
        var que: dynamic = cpp_uninitialized();
        range(i, 0, passed.size());
        if (passed[i])
        {
          que.push(i);
        }
        while ((!que.empty()))
        {
          var s: dynamic = que.front();
          que.pop();
          for (var eid: dynamic in f_cache[s])
          {
            if (((((es[eid].c == c) || (es[eid].c == cpp_char(".")))) && (!npassed[es[eid].t])))
            {
              npassed[es[eid].t] = true;
              que.push(es[eid].t);
            }
          }
        }
        range(i, 0, N);
        if (npassed[i])
        {
          for (var s: dynamic in trans[i][26])
          {
            npassed[s] = true;
          }
        }
        range(s, 0, npassed.size());
        if (npassed[s])
        {
          var alleps: dynamic = true;
          for (var eid: dynamic in f_cache[s])
          {
            alleps &= (es[eid].c == cpp_char("-"));
          }
          if ((alleps && (t != s)))
          {
            continue;
          }
          trans[st][(c - cpp_char("A"))].push_back(s);
        }
      }
    }
}

var ONE: dynamic = NFA(1);

class REGtoNFA
{
  func pattern(s: dynamic, i: dynamic, e: dynamic) -> dynamic
  {
      var p: dynamic = simple(s, i, e);
      while (((i < e) && (s[i] == cpp_char("|"))))
      {
        i += 1;
        var q: dynamic = simple(s, i, e);
        p = NFA.Union(p, q);
      }
      return p;
    }
  func simple(s: dynamic, i: dynamic, e: dynamic) -> dynamic
  {
      var p: dynamic = basic(s, i, e);
      while (((i < e) && (s[i] != cpp_char("|"))))
      {
        var q: dynamic = basic(s, i, e);
        p = NFA.Concat(p, q);
      }
      return p;
    }
  func basic(s: dynamic, i: dynamic, e: dynamic) -> dynamic
  {
      var p: dynamic = elementary(s, i, e);
      if (((i < cpp_cast(s.size())) && (s[i] == cpp_char("*"))))
      {
        i += 1;
        p = NFA.Star(p);
      }
      return p;
    }
  func elementary(s: dynamic, i: dynamic, e: dynamic) -> dynamic
  {
      var p: dynamic = cpp_uninitialized();
      if ((s[i] == cpp_char("(")))
      {
        i += 1;
        var d: dynamic = 1;
        {
          var j: dynamic = i;
          while ((j < e))
          {
            if ((s[j] == cpp_char("(")))
            {
              d += 1;
            }
            if ((s[j] == cpp_char(")")))
            {
              d -= 1;
            }
            if ((d == 0))
            {
              e = j;
              break;
            }
            j += 1;
          }
        }
        p = pattern(s, i, e);
        i += 1;
      } else
      {
        p = cpp_new(2);
        p->add_edge(s[i], p->s, p->t);
        i += 1;
      }
      return p;
    }
}

var rc: dynamic = 0;

class Main
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var res: dynamic = cpp_uninitialized();
  var NFAs: dynamic = cpp_uninitialized();
  var stats: dynamic = cpp_uninitialized();
  func dfs(y: dynamic, x: dynamic, c: dynamic) -> dynamic
  {
      if ((rc > 1))
      {
        return;
      }
      if ((y == h))
      {
        rc += c;
        res = tmp;
        return;
      }
      var nstats: dynamic = cpp_construct(2, vector(26));
      var nfai: dynamic = [y, (h + x)];
      range(ni, 0, 2);
      {
        var nc: dynamic = cpp_char("A");
        while ((nc <= cpp_char("Z")))
        {
          for (var s: dynamic in stats[nfai[ni]])
          {
            for (var t: dynamic in NFAs[nfai[ni]].trans[s][(nc - cpp_char("A"))])
            {
              nstats[ni][(nc - cpp_char("A"))].push_back(t);
            }
          }
          UNIQUE(nstats[ni][(nc - cpp_char("A"))]);
          nc += 1;
        }
      }
      var gid: dynamic = cpp_construct(26);
      iota(ALL(gid), 0);
      {
        var c1: dynamic = cpp_char("A");
        while ((c1 <= cpp_char("Z")))
        {
          {
            var c2: dynamic = cpp_char("A");
            while ((c2 < c1))
            {
              if ((gid[(c2 - cpp_char("A"))] == (c2 - cpp_char("A"))))
              {
                if (((nstats[0][(c1 - cpp_char("A"))].size() == nstats[0][(c2 - cpp_char("A"))].size()) && (nstats[1][(c1 - cpp_char("A"))].size() == nstats[1][(c2 - cpp_char("A"))].size())))
                {
                  var eq: dynamic = true;
                  range(ni, 0, 2);
                  {
                    range(i, 0, nstats[ni][(c1 - cpp_char("A"))].size());
                    {
                      eq &= (nstats[ni][(c1 - cpp_char("A"))][i] == nstats[ni][(c2 - cpp_char("A"))][i]);
                      if ((!eq))
                      {
                        break;
                      }
                    }
                    if ((!eq))
                    {
                      break;
                    }
                  }
                  if (eq)
                  {
                    gid[(c1 - cpp_char("A"))] = gid[(c2 - cpp_char("A"))];
                    break;
                  }
                }
              }
              c2 += 1;
            }
          }
          c1 += 1;
        }
      }
      var gc: dynamic = cpp_construct(26);
      range(i, 0, 26)[gid[i]] += 1;
      {
        var c: dynamic = cpp_char("A");
        while ((c <= cpp_char("Z")))
        {
          if ((gid[(c - cpp_char("A"))] == (c - cpp_char("A"))))
          {
            tmp[y][x] = c;
            var tmpOK: dynamic = true;
            {
              var ok: dynamic = false;
              for (var s: dynamic in nstats[0][(c - cpp_char("A"))])
              {
                ok |= NFAs[y].reachable[s][((w - 1) - x)];
              }
              tmpOK &= ok;
            }
            {
              var ok: dynamic = false;
              for (var s: dynamic in nstats[1][(c - cpp_char("A"))])
              {
                ok |= NFAs[(h + x)].reachable[s][((h - 1) - y)];
              }
              tmpOK &= ok;
            }
            if ((x == (w - 1)))
            {
              var ok: dynamic = false;
              for (var s: dynamic in nstats[0][(c - cpp_char("A"))])
              {
                ok |= (s == NFAs[y].t);
              }
              tmpOK &= ok;
            }
            if ((y == (h - 1)))
            {
              var ok: dynamic = false;
              for (var s: dynamic in nstats[1][(c - cpp_char("A"))])
              {
                ok |= (s == NFAs[(h + x)].t);
              }
              tmpOK &= ok;
            }
            if (tmpOK)
            {
              var tmpp: dynamic = stats[y];
              var tmpq: dynamic = stats[(h + x)];
              stats[y] = nstats[0][(c - cpp_char("A"))];
              stats[(h + x)] = nstats[1][(c - cpp_char("A"))];
              if (((x + 1) < w))
              {
                dfs(y, (x + 1), gc[(c - cpp_char("A"))]);
              } else
              {
                dfs((y + 1), 0, gc[(c - cpp_char("A"))]);
              }
              stats[y] = tmpp;
              stats[(h + x)] = tmpq;
            }
          }
          c += 1;
        }
      }
    }
  func run() -> dynamic
  {
      while (true)
      {
        read(h, w);
        if (((h == 0) && (w == 0)))
        {
          break;
        }
        rc = 0;
        tmp = cpp_assign(res, "=", vector(h, string_cpp(w, cpp_char("-"))));
        var ss: dynamic = cpp_construct((h + w));
        read(ss);
        NFAs = vector((h + w));
        range(i, 0, (h + w));
        {
          var s: dynamic = "";
          range(j, 1, (ss[i].size() - 1)) += ss[i][j];
          var cur: dynamic = 0;
          var e: dynamic = s.size();
          NFAs[i] = (*REGtoNFA.pattern(s, cur, e));
        }
        range(i, 0, (h + w));
        {
          NFAs[i].create_reachable();
          NFAs[i].create_trans();
        }
        stats = vector((h + w));
        range(i, 0, (h + w))[i].push_back(NFAs[i].s);
        dfs(0, 0, 1);
        if ((rc > 1))
        {
          write("ambiguous", "\n");
        } else if ((rc == 0))
        {
          write("none", "\n");
        } else
        {
          ((range(i, 0, h) << res[i]) << endl);
        }
      }
    }
}

func main() -> dynamic
{
  write(fixed, setprecision(20));
  cin.tie(0);
  ios.sync_with_stdio(false);
  Main().run();
  return 0;
}
