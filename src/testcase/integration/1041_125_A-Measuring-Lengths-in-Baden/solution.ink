// Translated from solution.cpp.

func token(a: dynamic)
{
  var w: dynamic;
  a.push_back(cpp_char(" "));
  while ((!a.empty()))
  {
    w.push_back(a.substr(0, a.find(" ")));
    a = a.substr((a.find(" ") + 1), (a.size() - 1));
  }
  return w;
}

var mapik: dynamic;

var amapik: dynamic;

func dodaj(a: dynamic)
{
  if ((mapik.count(a) == 0))
  {
    mapik[a] = (mapik.size() - 1);
    amapik.push_back(a);
  }
  return mapik[a];
}

var tmp_str = cpp_array(1000);

func scanf_string()
{
  scanf("%s", tmp_str);
  return tmp_str;
}

var N = 1000;

var n: dynamic;

func main()
{
  scanf("%d", (&n));
  var inches = (((n + 1)) / 3);
  printf("%d %d\n", (inches / 12), (inches % 12));
  return 0;
}
