// Translated from solution.cpp.

func FOR(i: dynamic, k: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (int)(k); i < (int)(n); i++)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <bits/std");
}

func MS(m: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

var MOD = 1000000007;

var INF = (MOD + 1);

var EPS = 1e-12;

func chmin(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", min(a, b));
}

func chmax(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", max(a, b));
}

enum yaku
{
  hc,
  op,
  tp,
  tc,
  st,
  fl,
  fh,
  fc,
  sf,
  rsf
}

func parse(s: dynamic)
{
  var suit: dynamic;
  if ((s[0] == cpp_char("S")))
  {
    suit = 0;
  } else if ((s[0] == cpp_char("H")))
  {
    suit = 1;
  } else if ((s[0] == cpp_char("D")))
  {
    suit = 2;
  } else if ((s[0] == cpp_char("C")))
  {
    suit = 3;
  }
  var n: dynamic;
  if (isdigit(s[1]))
  {
    n = (s[1] - cpp_char("0"));
  } else if ((s[1] == cpp_char("A")))
  {
    n = 1;
  } else if ((s[1] == cpp_char("T")))
  {
    n = 10;
  } else if ((s[1] == cpp_char("J")))
  {
    n = 11;
  } else if ((s[1] == cpp_char("Q")))
  {
    n = 12;
  } else if ((s[1] == cpp_char("K")))
  {
    n = 13;
  }
  return card(n, suit);
}

var pat: dynamic;

func init()
{
  var v = [0, 1, 2, 3, 4, 5, 6];
  while (true)
  {
    var tmp: dynamic;
    REP(i, 5).push_back(v[i]);
    sort(ALL(tmp));
    pat.push_back(tmp);
    if (!((next_permutation(ALL(v)))))
    {
      break;
    }
  }
  sort(ALL(pat));
  pat.erase(unique(ALL(pat)), pat.end());
}

func is_straight(cards: dynamic)
{
  var v: dynamic;
  for (var i in cards)
  {
    v.push_back(i.first);
  }
  if ((v == [1, 10, 11, 12, 13]))
  {
    return 14;
  }
  REP(i, 4);
  {
    if (((v[(i + 1)] - v[i]) != 1))
    {
      return 0;
    }
  }
  return v.back();
}

func calc(cards: dynamic)
{
  var straight = is_straight(cards);
  var flush = true;
  var rank = cpp_construct(13);
  REP(i, 5);
  {
    rank[(cards[i].first - 1)] += 1;
    if (((i != 4) && (cards[(i + 1)].second != cards[i].second)))
    {
      flush = false;
    }
  }
  if ((straight && flush))
  {
    if ((straight == 14))
    {
      return hand(rsf, []);
    } else
    {
      return hand(sf, [straight]);
    }
  }
  var two: dynamic;
  var three: dynamic;
  var four: dynamic;
  REP(i, 13);
  {
    if ((rank[i] == 2))
    {
      two.push_back((i + 1));
    }
    if ((rank[i] == 3))
    {
      three.push_back((i + 1));
    }
    if ((rank[i] == 4))
    {
      four.push_back((i + 1));
    }
  }
  if ((four.size() == 1))
  {
    var v = four;
    if ((rank[0] == 1))
    {
      v.push_back(1);
    }
    {
      var i = 12;
      while ((i > 0))
      {
        if ((rank[i] == 1))
        {
          v.push_back((i + 1));
        }
        i -= 1;
      }
    }
    REP(i, v.size());
    {
      if ((v[i] == 1))
      {
        v[i] = 14;
      }
    }
    return hand(fc, v);
  }
  if (((three.size() == 1) && (two.size() == 1)))
  {
    var v = [three[0], two[0]];
    REP(i, v.size());
    {
      if ((v[i] == 1))
      {
        v[i] = 14;
      }
    }
    return hand(fh, v);
  }
  var tmp: dynamic;
  for (var i in cards)
  {
    if ((i.first == 1))
    {
      tmp.push_back(14);
    } else
    {
      tmp.push_back(i.first);
    }
  }
  sort(tmp.rbegin(), tmp.rend());
  if (flush)
  {
    return hand(fl, tmp);
  }
  if (straight)
  {
    return hand(st, [straight]);
  }
  if ((three.size() == 1))
  {
    var v = three;
    if ((rank[0] == 1))
    {
      v.push_back(1);
    }
    {
      var i = 12;
      while ((i > 0))
      {
        if ((rank[i] == 1))
        {
          v.push_back((i + 1));
        }
        i -= 1;
      }
    }
    REP(i, v.size());
    {
      if ((v[i] == 1))
      {
        v[i] = 14;
      }
    }
    return hand(tc, v);
  }
  if ((two.size() == 2))
  {
    sort(two.rbegin(), two.rend());
    var v = two;
    if ((v[1] == 1))
    {
      swap(v[0], v[1]);
    }
    if ((rank[0] == 1))
    {
      v.push_back(1);
    }
    {
      var i = 12;
      while ((i > 0))
      {
        if ((rank[i] == 1))
        {
          v.push_back((i + 1));
        }
        i -= 1;
      }
    }
    REP(i, v.size());
    {
      if ((v[i] == 1))
      {
        v[i] = 14;
      }
    }
    return hand(tp, v);
  }
  if ((two.size() == 1))
  {
    var v = two;
    if ((rank[0] == 1))
    {
      v.push_back(1);
    }
    {
      var i = 12;
      while ((i > 0))
      {
        if ((rank[i] == 1))
        {
          v.push_back((i + 1));
        }
        i -= 1;
      }
    }
    REP(i, v.size());
    {
      if ((v[i] == 1))
      {
        v[i] = 14;
      }
    }
    return hand(op, v);
  }
  return hand(hc, tmp);
}

func main()
{
  cin.sync_with_stdio(false);
  write(fixed, setprecision(10));
  init();
  var v = cpp_construct(7);
  while (cpp_comma((cin >> v[0]), (v[0] != "#")))
  {
    var used: dynamic;
    REP(i, 6);
    read(v[(i + 1)]);
    var my_cards: dynamic;
    var op_cards: dynamic;
    REP(i, 2).push_back(parse(v[i]));
    REP(i, 2).push_back(parse(v[(i + 2)]));
    REP(i, 3);
    {
      my_cards.push_back(parse(v[(i + 4)]));
      op_cards.push_back(parse(v[(i + 4)]));
    }
    REP(i, 5);
    {
      used.insert(my_cards[i]);
      used.insert(op_cards[i]);
    }
    var cnt = 0;
    var cnt2 = 0;
    REP(suit1, 4);
    FOR(num1, 1, 14);
    REP(suit2, 4);
    FOR(num2, 1, 14);
    {
      if (((suit1 == suit2) && (num1 == num2)))
      {
        continue;
      }
      if ((used.count(c1) || used.count(c2)))
      {
        continue;
      }
      var t_my = my_cards;
      var t_op = op_cards;
      t_my.push_back(c1);
      t_my.push_back(c2);
      t_op.push_back(c2);
      t_op.push_back(c1);
      var my_hand = cpp_construct(hc, []);
      var op_hand = cpp_construct(hc, []);
      for (var use in pat)
      {
        var t_my_cards: dynamic;
        var t_op_cards: dynamic;
        for (var i in use)
        {
          t_my_cards.push_back(t_my[i]);
          t_op_cards.push_back(t_op[i]);
        }
        sort(ALL(t_my_cards));
        sort(ALL(t_op_cards));
        chmax(my_hand, calc(t_my_cards));
        chmax(op_hand, calc(t_op_cards));
      }
      if ((my_hand > op_hand))
      {
        cnt += 1;
      }
      cnt2 += 1;
    }
    write((cpp_cast(cnt) / cnt2), "\n");
  }
  return 0;
}
