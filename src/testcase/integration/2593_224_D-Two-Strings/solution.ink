// Translated from solution.cpp.

func LP(S: dynamic, T: dynamic)
{
  var M = int_cpp((T).size());
  var last_pos_in_T = cpp_construct(26, 0);
  var N = int_cpp((S).size());
  var res = cpp_construct(N, -1);
  {
    var i = 0;
    var j = 0;
    while ((i < N))
    {
      var ch_id = (S[i] - cpp_char("a"));
      if (((j < M) && (S[i] == T[j])))
      {
        res[i] = cpp_update(j, "++");
        last_pos_in_T[ch_id] = j;
      } else
      {
        res[i] = last_pos_in_T[ch_id];
      }
      i += 1;
    }
  }
  return res;
}

func main(argc: dynamic, argv: dynamic)
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var S: dynamic;
  var T: dynamic;
  read(S);
  read(T);
  var L = LP(S, T);
  reverse(S.begin(), S.end());
  reverse(T.begin(), T.end());
  var R = LP(S, T);
  reverse(R.begin(), R.end());
  for (var x in R)
  {
    x = ((int_cpp((T).size()) - x) + 1);
  }
  var ok = true;
  {
    var i = 0;
    while ((i < int_cpp((S).size())))
    {
      if ((((L[i] < 0) || (R[i] < 0)) || (L[i] < R[i])))
      {
        ok = false;
        break;
      }
      i += 1;
    }
  }
  write((if (ok) "Yes" else "No"), "\n");
  return 0;
}
