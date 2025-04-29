use std::cmp;
use std::fmt;

pub struct UnknownSymbolError {
    symbol: char,
}

impl fmt::Display for UnknownSymbolError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Unknown card symbol '{}'", self.symbol)
    }
}

impl fmt::Debug for UnknownSymbolError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Unknown card symbol '{}'", self.symbol)
    }
}

#[derive(PartialEq, Eq, Clone, Debug)]
pub enum Card {
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace,
}

impl Card {
    pub fn value(&self, use_joker: bool) -> u8 {
        match self {
            Card::Two => 2,
            Card::Three => 3,
            Card::Four => 4,
            Card::Five => 5,
            Card::Six => 6,
            Card::Seven => 7,
            Card::Eight => 8,
            Card::Nine => 9,
            Card::Ten => 10,
            Card::Jack => {
                if use_joker {
                    1
                } else {
                    11
                }
            }
            Card::Queen => 12,
            Card::King => 13,
            Card::Ace => 14,
        }
    }

    pub fn from_char(c: char) -> Result<Card, UnknownSymbolError> {
        match c {
            '2' => Ok(Card::Two),
            '3' => Ok(Card::Three),
            '4' => Ok(Card::Four),
            '5' => Ok(Card::Five),
            '6' => Ok(Card::Six),
            '7' => Ok(Card::Seven),
            '8' => Ok(Card::Eight),
            '9' => Ok(Card::Nine),
            'T' => Ok(Card::Ten),
            'J' => Ok(Card::Jack),
            'Q' => Ok(Card::Queen),
            'K' => Ok(Card::King),
            'A' => Ok(Card::Ace),
            _ => Err(UnknownSymbolError { symbol: c }),
        }
    }
}

#[derive(Debug)]
pub enum HandType {
    HighCard,
    OnePair,
    TwoPair,
    ThreeOfAKind,
    FullHouse,
    FourOfAKind,
    FiveOfAKind,
}

impl HandType {
    pub fn score(&self) -> u8 {
        match self {
            HandType::HighCard => 0,
            HandType::OnePair => 1,
            HandType::TwoPair => 2,
            HandType::ThreeOfAKind => 3,
            HandType::FullHouse => 4,
            HandType::FourOfAKind => 5,
            HandType::FiveOfAKind => 6,
        }
    }
}

#[derive(Debug)]
pub struct Hand {
    bid: usize,
    strength: HandType,
    cards: [Card; 5],
    counter: Vec<(Card, usize)>,
}

impl Hand {
    pub fn create(cards: [Card; 5], bid: usize) -> Self {
        let counter = count_cards(&cards);
        Hand {
            bid,
            strength: compute_strength(&counter),
            cards,
            counter,
        }
    }
}

pub fn compare_hand(hand: &Hand, other: &Hand, use_joker: bool) -> cmp::Ordering {
    let score1 = hand.strength.score();
    let score2 = other.strength.score();
    if score1 != score2 {
        return score1.cmp(&score2);
    }
    for i in 0..5 {
        let value1 = hand.cards[i].value(use_joker);
        let value2 = other.cards[i].value(use_joker);
        if value1 != value2 {
            return value1.cmp(&value2);
        }
    }
    cmp::Ordering::Equal
}

pub fn compute_strength(counter: &[(Card, usize)]) -> HandType {
    let n1 = counter[0].1;
    let n2 = match counter.iter().nth(1) {
        Some((_, n)) => *n,
        None => 0,
    };
    match (n1, n2) {
        (5, 0) => HandType::FiveOfAKind,
        (4, 1) => HandType::FourOfAKind,
        (3, 2) => HandType::FullHouse,
        (3, 1) => HandType::ThreeOfAKind,
        (2, 2) => HandType::TwoPair,
        (2, 1) => HandType::OnePair,
        _ => HandType::HighCard,
    }
}

pub fn count_cards(cards: &[Card]) -> Vec<(Card, usize)> {
    let mut result: Vec<(Card, usize)> = Vec::with_capacity(5);
    for card in cards.iter() {
        match result.iter().position(|x| x.0 == *card) {
            Some(i) => {
                result[i].1 += 1;
            }
            None => {
                result.push((card.clone(), 1));
            }
        }
    }
    result.sort_by(|a, b| b.1.cmp(&a.1));
    result
}

pub fn compute_score(hands: &[Hand]) -> usize {
    hands
        .iter()
        .enumerate()
        .map(|(i, hand)| (i + 1) * hand.bid)
        .sum()
}

pub fn apply_joker(hand: &mut Hand) {
    let ijkr = hand.counter.iter().position(|x| x.0 == Card::Jack);
    let ijkr = match ijkr {
        None => return,
        Some(i) => {
            if hand.counter.len() == 1 {
                return;
            } else {
                i
            }
        }
    };
    let imax = hand
        .counter
        .iter()
        .position(|cc| cc.0 != Card::Jack)
        .unwrap();
    hand.counter[imax].1 += hand.counter[ijkr].1;
    hand.counter[ijkr].1 = 0;
    hand.counter.sort_by(|a, b| b.1.cmp(&a.1));
    hand.strength = compute_strength(&hand.counter);
}
