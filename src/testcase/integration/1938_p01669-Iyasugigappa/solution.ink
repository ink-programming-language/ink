// Translated from solution.cpp.

var INF = 1e9;

var memo: dynamic;

var memo2: dynamic;

var next_st: dynamic;

var get_score: dynamic;

var card = cpp_array(2, 3);

func make_array(a: dynamic, b: dynamic, c: dynamic)
{
  return [[a, b, c]];
}

enum cpp_enum_1
{
  frog,
  kappa,
  weasel
}

func rec(fld: dynamic, action: dynamic, turn: dynamic, k: dynamic)
{
  if ((fld.size() == 0))
  {
    return make_array(0, 0, 0);
  }
  var now = make_tuple(fld, action, turn);
  if (((if (k) memo2 else memo).count(now) == 1))
  {
    return (if (k) memo2 else memo)[now];
  }
  var res = (if (k) memo2 else memo)[now];
  res = make_array(0, 0, (-INF));
  var sum = 0;
  {
    var i = 0;
    while ((i < 2))
    {
      if ((action & ((1 << (((2 * turn) + i))))))
      {
        sum += card[turn][i];
      }
      i += 1;
    }
  }
  if (((!k) || (turn == kappa)))
  {
    var cand: dynamic;
    var comp: dynamic;
    var next_fld: dynamic;
    var next_action: dynamic;
    var add_score: dynamic;
    {
      var f = fld;
      var score = (*f.begin());
      f.erase(f.begin());
      var t = rec(f, action, (((turn + 1)) % 3), k);
      t[turn] += score;
      cand.push_back(move(t));
      next_fld.push_back(f);
      next_action.push_back(action);
      add_score.push_back(score);
      if (((turn == kappa) && k))
      {
        comp.emplace_back((-cand.back()[frog]), sum);
      } else
      {
        comp.emplace_back(cand.back()[turn], sum);
      }
    }
    {
      var i = 0;
      while ((i < 2))
      {
        if (((action & ((1 << (((2 * turn) + i))))) && (card[turn][i] <= fld.size())))
        {
          var nxt_act = (action & (~((1 << (((2 * turn) + i))))));
          var f = fld;
          var score = (*(((f.begin() + card[turn][i]) - 1)));
          f.erase(((f.begin() + card[turn][i]) - 1));
          var t = rec(f, nxt_act, (((turn + 1)) % 3), k);
          t[turn] += score;
          cand.push_back(move(t));
          next_fld.push_back(f);
          next_action.push_back(nxt_act);
          add_score.push_back(score);
          if (((turn == kappa) && k))
          {
            comp.emplace_back((-cand.back()[frog]), (sum - card[turn][i]));
          } else
          {
            comp.emplace_back(cand.back()[turn], (sum - card[turn][i]));
          }
        }
        i += 1;
      }
    }
    var idx = (max_element(comp.begin(), comp.end()) - comp.begin());
    if ((!k))
    {
      next_st[now] = make_tuple(next_fld[idx], next_action[idx], (((turn + 1)) % 3));
      get_score[now] = add_score[idx];
    }
    return cpp_assign(res, "=", cand[idx]);
  } else
  {
    var f: dynamic;
    var a: dynamic;
    var t: dynamic;
    tie(f, a, t) = next_st[now];
    var tmp = rec(f, a, t, true);
    tmp[turn] += get_score[now];
    return cpp_assign(res, "=", tmp);
  }
}

func main()
{
  var fld = cpp_construct(12);
  {
    var i = 11;
    while ((i >= 0))
    {
      read(fld[i]);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = 0;
        while ((j < 2))
        {
          read(card[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  rec(fld, 0b111111, 0, false);
  var res = rec(fld, 0b111111, 0, true);
  write(res[0], cpp_char(" "), res[1], cpp_char(" "), res[2], "\n");
}
