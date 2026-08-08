// Translated from solution.cpp.

var pb = cpp_expression("#include<");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);i++)");
}

var f = cpp_array(128);

func isAce11(hand: dynamic)
{
  var sc = 0;
  var acecnt = 0;
  rep(i, hand.size());
  {
    sc += f[hand[i]];
    if ((hand[i] == cpp_char("A")))
    {
      acecnt += 1;
    }
  }
  return (((acecnt > 0) && (sc <= 11)));
}

func score(hand: dynamic)
{
  var sc = 0;
  var acecnt = 0;
  rep(i, hand.size());
  {
    sc += f[hand[i]];
    if ((hand[i] == cpp_char("A")))
    {
      acecnt += 1;
    }
  }
  if (((acecnt > 0) && (sc <= 11)))
  {
    sc += 10;
  }
  return sc;
}

func main()
{
  f[cpp_char("A")] = 1;
  f[cpp_char("2")] = 2;
  f[cpp_char("3")] = 3;
  f[cpp_char("4")] = 4;
  f[cpp_char("5")] = 5;
  f[cpp_char("6")] = 6;
  f[cpp_char("7")] = 7;
  f[cpp_char("8")] = 8;
  f[cpp_char("9")] = 9;
  f[cpp_char("T")] = 10;
  f[cpp_char("J")] = 10;
  f[cpp_char("Q")] = 10;
  f[cpp_char("K")] = 10;
  var T: dynamic;
  scanf("%d", (&T));
  while (cpp_update(T, "--"))
  {
    var hand = cpp_construct(2);
    scanf(" %c %c", (&hand[0]), (&hand[1]));
    var card = cpp_array(8);
    rep(i, 8);
    scanf(" %c", (card + i));
    var sc = score(hand);
    if ((sc == 21))
    {
      puts("blackjack");
      continue;
    }
    {
      var i = 0;
      while (true)
      {
        sc = score(hand);
        if ((sc > 21))
        {
          puts("bust");
          break;
        }
        if (((sc > 17) || (((!isAce11(hand)) && (sc == 17)))))
        {
          printf("%d\n", sc);
          break;
        }
        hand.pb(card[i]);
        i += 1;
      }
    }
  }
  return 0;
}
