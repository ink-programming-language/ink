// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var v = cpp_construct(9);
  v[0].first = cpp_char("6");
  v[1].first = cpp_char("7");
  v[2].first = cpp_char("8");
  v[3].first = cpp_char("9");
  v[4].first = cpp_char("T");
  v[5].first = cpp_char("J");
  v[6].first = cpp_char("Q");
  v[7].first = cpp_char("K");
  v[8].first = cpp_char("A");
  v[0].second = 6;
  v[1].second = 7;
  v[2].second = 8;
  v[3].second = 9;
  v[4].second = 10;
  v[5].second = 11;
  v[6].second = 12;
  v[7].second = 13;
  v[8].second = 14;
  var trump: dynamic;
  var s1: dynamic;
  var s2: dynamic;
  var valu_s1: dynamic;
  var valu_s2: dynamic;
  read(trump);
  read(s1, s2);
  if (((s1[1] == trump) && (s2[1] != trump)))
  {
    write("YES", "\n");
  } else if ((s1[1] == s2[1]))
  {
    {
      var i = 0;
      while ((i < 9))
      {
        if ((s1[0] == v[i].first))
        {
          valu_s1 = v[i].second;
        }
        if ((s2[0] == v[i].first))
        {
          valu_s2 = v[i].second;
        }
        i += 1;
      }
    }
    if ((valu_s1 > valu_s2))
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
    }
  } else
  {
    write("NO", "\n");
  }
  return 0;
}
