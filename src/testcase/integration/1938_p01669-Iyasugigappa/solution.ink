// Translated from solution.cpp.

var INF: dynamic = 1e9;

var memo: dynamic = cpp_uninitialized();

var memo2: dynamic = cpp_uninitialized();

var next_st: dynamic = cpp_uninitialized();

var get_score: dynamic = cpp_uninitialized();

var card: dynamic = cpp_array(2, 3);

func make_array(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  return [[a, b, c]];
}

enum cpp_enum_1
{
  enum_field frog;
  enum_field kappa;
  enum_field weasel;
}

func rec(fld: dynamic, action: dynamic, turn: dynamic, k: dynamic) -> dynamic
{
  if ((fld.size() == 0))
  {
    return make_array(0, 0, 0);
  }
  var now: dynamic = make_tuple(fld, action, turn);
  if ((( (k) ? memo2 : memo).count(now) == 1))
  {
    return ( (k) ? memo2 : memo)[now];
  }
  var res: dynamic = ( (k) ? memo2 : memo)[now];
  res = make_array(0, 0, (-INF));
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
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
    var cand: dynamic = cpp_uninitialized();
    var comp: dynamic = cpp_uninitialized();
    var next_fld: dynamic = cpp_uninitialized();
    var next_action: dynamic = cpp_uninitialized();
    var add_score: dynamic = cpp_uninitialized();
    {
      var f: dynamic = fld;
      var score: dynamic = (*f.begin());
      f.erase(f.begin());
      var t: dynamic = rec(f, action, (((turn + 1)) % 3), k);
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
      var i: dynamic = 0;
      while ((i < 2))
      {
        if (((action & ((1 << (((2 * turn) + i))))) && (card[turn][i] <= fld.size())))
        {
          var nxt_act: dynamic = (action & (~((1 << (((2 * turn) + i))))));
          var f: dynamic = fld;
          var score: dynamic = (*(((f.begin() + card[turn][i]) - 1)));
          f.erase(((f.begin() + card[turn][i]) - 1));
          var t: dynamic = rec(f, nxt_act, (((turn + 1)) % 3), k);
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
    var idx: dynamic = (max_element(comp.begin(), comp.end()) - comp.begin());
    if ((!k))
    {
      next_st[now] = make_tuple(next_fld[idx], next_action[idx], (((turn + 1)) % 3));
      get_score[now] = add_score[idx];
    }
    return cpp_assign(res, "=", cand[idx]);
  } else
  {
    var f: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    tie(f, a, t) = next_st[now];
    var tmp: dynamic = rec(f, a, t, true);
    tmp[turn] += get_score[now];
    return cpp_assign(res, "=", tmp);
  }
}

func main() -> dynamic
{
  var fld: dynamic = cpp_construct(12);
  {
    var i: dynamic = 11;
    while ((i >= 0))
    {
      read(fld[i]);
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      {
        var j: dynamic = 0;
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
  var res: dynamic = rec(fld, 0b111111, 0, true);
  write(res[0], cpp_char(" "), res[1], cpp_char(" "), res[2], "\n");
}
