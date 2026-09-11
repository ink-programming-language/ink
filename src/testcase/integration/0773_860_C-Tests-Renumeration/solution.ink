// Translated from solution.cpp.

var INF: dynamic = 1791791791;

var INFLL: dynamic = 1791791791791791791;

class FastIO
{
  var cbuf: dynamic = cpp_array((input_buf_size + 1));
  var icur: dynamic = cpp_uninitialized();
  func go_to_next_token() -> dynamic
  {
      while (((cbuf[icur] == cpp_char(" ")) || (cbuf[icur] == cpp_char("\n"))))
      {
        icur += 1;
      }
      while ((cbuf[icur] == 0))
      {
        icur = 0;
        if ((fgets(cbuf, cpp_sizeof((cbuf)), stdin) != cbuf))
        {
          return false;
        }
        while (((cbuf[icur] == cpp_char(" ")) || (cbuf[icur] == cpp_char("\n"))))
        {
          icur += 1;
        }
      }
      return true;
    }
  func readString() -> dynamic
  {
      assert(go_to_next_token());
      var ans: dynamic = cpp_uninitialized();
      while ((((cbuf[icur] != cpp_char(" ")) && (cbuf[icur] != cpp_char("\n"))) && (cbuf[icur] != 0)))
      {
        ans.push_back(cbuf[cpp_update(icur, "++")]);
      }
      ans.shrink_to_fit();
      return ans;
    }
  func readInt() -> dynamic
  {
      assert(go_to_next_token());
      var x: dynamic = 0;
      var m: dynamic = (cbuf[icur] == cpp_char("-"));
      if (m)
      {
        icur += 1;
      }
      while (((cpp_char("0") <= cbuf[icur]) && (cbuf[icur] <= cpp_char("9"))))
      {
        x *= 10;
        x += ((cbuf[icur] - cpp_char("0")));
        icur += 1;
      }
      if (m)
      {
        x = (-x);
      }
      return x;
    }
  func seekEof() -> dynamic
  {
      return (!go_to_next_token());
    }
  var obuf: dynamic = cpp_array((output_buf_size + 1));
  var ocur: dynamic = cpp_uninitialized();
  func write_string(str: dynamic, sz: dynamic = 0) -> dynamic
  {
      if ((sz == 0))
      {
        sz = strlen(str);
      }
      if (((ocur + sz) > output_buf_size))
      {
        fputs(obuf, stdout);
        fputs(str, stdout);
        ocur = 0;
        obuf[0] = 0;
        return;
      }
      strcpy((obuf + ocur), str);
      ocur += sz;
      obuf[ocur] = 0;
    }
  func writeInt(x: dynamic, sp: dynamic = true) -> dynamic
  {
      var buf: dynamic = cpp_array(21);
      var c: dynamic = 0;
      if ((x < 0))
      {
        buf[cpp_update(c, "++")] = cpp_char("-");
        x = (-x);
      }
      var s: dynamic = c;
      if ((x == 0))
      {
        buf[cpp_update(c, "++")] = cpp_char("0");
      }
      while ((x > 0))
      {
        buf[cpp_update(c, "++")] = (((x % 10)) + cpp_char("0"));
        x /= 10;
      }
      {
        var i: dynamic = 0;
        while (((2 * i) < (c - s)))
        {
          swap(buf[(s + i)], buf[((c - 1) - i)]);
          i += 1;
        }
      }
      buf[c] = 0;
      write_string(buf, c);
      if (sp)
      {
        write_string(" ", 1);
      }
    }
  func writeString(s: dynamic, space: dynamic = true) -> dynamic
  {
      write_string(s.c_str(), s.size());
      if (space)
      {
        write_string(" ", 1);
      }
    }
  func writeEndl() -> dynamic
  {
      write_string("\n", 1);
    }
  func flush() -> dynamic
  {
      fputs(obuf, stdout);
      ocur = 0;
      obuf[0] = 0;
    }
  var lflush: dynamic = cpp_uninitialized();
  func FastIO(local_flush: dynamic) -> dynamic
  {
      obuf[0] = 0;
      lflush = local_flush;
    }
  func cpp_destruct_FastIO() -> dynamic
  {
      fputs(obuf, stdout);
    }
}

var IO: dynamic = cpp_construct(true);

var n: dynamic = cpp_uninitialized();

var clast: dynamic = cpp_uninitialized();

var by_idx: dynamic = cpp_uninitialized();

var idx: dynamic = cpp_uninitialized();

func add_string(s: dynamic) -> dynamic
{
  if (idx.count(s))
  {
    return;
  } else
  {
    by_idx.push_back(s);
    idx[s] = cpp_update(clast, "++");
  }
}

var rnd: dynamic = cpp_construct(179);

func gen_random() -> dynamic
{
  var dist: dynamic = cpp_construct(cpp_char("a"), cpp_char("z"));
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((6)))))
    {
      ans.push_back(dist(rnd));
      i += 1;
    }
  }
  return ans;
}

func not_in_set() -> dynamic
{
  var t: dynamic = gen_random();
  while (idx.count(t))
  {
    t = gen_random();
  }
  return t;
}

func to_str(x: dynamic) -> dynamic
{
  var s: dynamic = cpp_array(10);
  sprintf(s, "%d", x);
  return s;
}

var ws_to: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = IO.readInt_int();
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      add_string(to_str((i + 1)));
      i += 1;
    }
  }
  var inp: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      var s: dynamic = IO.readString();
      add_string(s);
      var b: dynamic = IO.readInt_int();
      inp.push_back(make_pair(s, b));
      ws_to[s] = b;
      i += 1;
    }
  }
  var w: dynamic = not_in_set();
  add_string(w);
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      filled[idx[inp[i].first]] = true;
      if ((idx[inp[i].first] < n))
      {
        who[idx[inp[i].first]] = i;
      }
      i += 1;
    }
  }
  var numf: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      numf += inp[i].second;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      if (((idx[inp[i].first] < numf) && inp[i].second))
      {
        ok[idx[inp[i].first]] = true;
      }
      if ((((numf <= idx[inp[i].first]) && (idx[inp[i].first] < n)) && (!inp[i].second)))
      {
        ok[idx[inp[i].first]] = true;
      }
      i += 1;
    }
  }
  var wanted: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var bucket: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((n)))))
    {
      if ((!filled[i]))
      {
        bucket = i;
      }
      if ((filled[i] && (!ok[i])))
      {
        wanted.push_back(i);
      }
      i += 1;
    }
  }
  if ((bucket == -1))
  {
    var it: dynamic = find((ok).begin(), (ok).end(), false);
    if ((it == ok.end()))
    {
      IO.writeInt(0);
      IO.writeEndl();
      return 0;
    }
    bucket = (it - ok.begin());
    ans.push_back(make_pair(by_idx[bucket], w));
    filled[bucket] = false;
    filled.back() = true;
    wanted.erase(find((wanted).begin(), (wanted).end(), bucket));
    ws_to[w] = ws_to[by_idx[bucket]];
  }
  var wfa: dynamic = cpp_uninitialized();
  var wfb: dynamic = cpp_uninitialized();
  for (var u: dynamic in wanted)
  {
    if ((u < numf))
    {
      wfa.push(u);
    } else
    {
      wfb.push(u);
    }
  }
  while ((wfa.size() && wfb.size()))
  {
    var a: dynamic = cpp_uninitialized();
    if ((bucket < numf))
    {
      a = wfb.front();
      wfb.pop();
    } else
    {
      a = wfa.front();
      wfa.pop();
    }
    ans.push_back(make_pair(by_idx[a], by_idx[bucket]));
    filled[bucket] = true;
    filled[a] = false;
    bucket = a;
  }
  var free_a: dynamic = cpp_uninitialized();
  var free_b: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((numf)))))
    {
      if ((!filled[i]))
      {
        free_a.push(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = cpp_cast((numf));
    while ((i < (cpp_cast((n)))))
    {
      if ((!filled[i]))
      {
        free_b.push(i);
      }
      i += 1;
    }
  }
  while (wfa.size())
  {
    var a: dynamic = wfa.front();
    wfa.pop();
    var b: dynamic = free_b.front();
    free_b.pop();
    ans.push_back(make_pair(by_idx[a], by_idx[b]));
    filled[a] = false;
    filled[b] = true;
  }
  while (wfb.size())
  {
    var a: dynamic = wfb.front();
    wfb.pop();
    var b: dynamic = free_a.front();
    free_a.pop();
    ans.push_back(make_pair(by_idx[a], by_idx[b]));
    filled[a] = false;
    filled[b] = true;
  }
  free_a = queue();
  free_b = queue();
  {
    var i: dynamic = 0;
    while ((i < (cpp_cast((numf)))))
    {
      if ((!filled[i]))
      {
        free_a.push(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = cpp_cast((numf));
    while ((i < (cpp_cast((n)))))
    {
      if ((!filled[i]))
      {
        free_b.push(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = cpp_cast((n));
    while ((i < (cpp_cast((clast)))))
    {
      if (filled[i])
      {
        var a: dynamic = cpp_uninitialized();
        if (ws_to[by_idx[i]])
        {
          a = free_a.front();
          free_a.pop();
        } else
        {
          a = free_b.front();
          free_b.pop();
        }
        ans.push_back(make_pair(by_idx[i], by_idx[a]));
        filled[i] = false;
        filled[a] = true;
      }
      i += 1;
    }
  }
  IO.writeInt(ans.size(), 0);
  IO.writeEndl();
  for (var p: dynamic in ans)
  {
    IO.writeString("move");
    IO.writeString(p.first);
    IO.writeString(p.second, 0);
    IO.writeEndl();
  }
  return 0;
}
